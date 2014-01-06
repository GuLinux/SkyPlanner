#ifndef CATALOGS_PARSING_DB_HELPER
#define CATALOGS_PARSING_DB_HELPER

#include <QCoreApplication>
#include <QStringList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <map>
#include <boost/concept_check.hpp>

class CatalogsImporter {
public:
  CatalogsImporter(const std::string &catalogue, int argc, char **argv);
  CatalogsImporter(const std::string &catalogue, QCoreApplication &app);
  ~CatalogsImporter();
  long long insertObject(const std::string &objectId, double rightAscension, double declination, double magnitude, double angularSize, int type);
  long long insertDenomination(std::string catalogueNumber, const std::string &name, const std::string &comment, long long objectId, int searchMode, const std::string &other_catalogues = std::string());
  long long findByName(const std::string &name);
  long long findByCatalog(const std::string &catalog,  const std::string &number);
  long long findByCatalog(const std::string &catalogAndNumber);
  long long findBy(const std::string &query, const std::map<std::string,QVariant> &bindValues);
private:
  QString catalogue;
  void init(QStringList arguments);
  long long lastInsertId(QSqlQuery &query, const QString &selectQuery, std::map<QString,QVariant> bindValues = {});
  QSqlDatabase db;
};

CatalogsImporter::CatalogsImporter( const std::string &catalogue, int argc, char **argv )
  : catalogue(QString::fromStdString(catalogue).trimmed())
{
  QCoreApplication app(argc, argv);
  init(app.arguments());
}

void __dumpQuery(const QString &where, const QSqlQuery &query) {
  qDebug() << where << ": Error running query " << query.lastQuery();
  for(auto v: query.boundValues().keys())
    qDebug() << "bound value: " << v << "=" << query.boundValues()[v];
  qDebug() << ": Last error: " << query.lastError().text();
}

#define dumpQuery(q) __dumpQuery(__PRETTY_FUNCTION__, q)


CatalogsImporter::CatalogsImporter( const std::string &catalogue, QCoreApplication &app )
  : catalogue(QString::fromStdString(catalogue).trimmed())
{
  init(app.arguments());
}

CatalogsImporter::~CatalogsImporter()
{
  bool committed = db.commit();
  qDebug() << __PRETTY_FUNCTION__ << ": Commit: " << committed;
  if(!committed) {
    qDebug() << __PRETTY_FUNCTION__ << ": Last error: " << db.lastError().text();
  }
}

void CatalogsImporter::init( QStringList arguments )
{
  qDebug() << __PRETTY_FUNCTION__ << ": Arguments: " << arguments;
  arguments.removeFirst();
  QString driver;
  QString username;
  QString password;
  QString database;
  QString hostname;
  for(int i=0; i<arguments.size(); i++) {
    auto assign = [&i, &arguments](const QString &option, QString &arg) {
    if(arguments[i] == option)
      arg = arguments[++i];
    };
    assign("-c", driver);
    assign("-u", username);
    assign("-p", password);
    assign("-d", database);
    assign("-h", hostname);
  }
  qDebug() << "Initializing database: driver=" << driver << ", username=" << username << ", password=" << password << ", database=" << database;
  db = QSqlDatabase::addDatabase(driver);
  if(!username.isNull())
    db.setUserName(username);
  if(!hostname.isNull())
    db.setHostName(hostname);
  if(!password.isNull())
    db.setPassword(password);
  if(!database.isNull())
    db.setDatabaseName(database);
  if(!db.open()) {
    std::stringstream error("Error opening database!\n");
    error << "Params:  driver=" << driver.toStdString() << ", username=" << username.toStdString() << ", password=" << password.toStdString() << ", database=" << database.toStdString();
    std::cerr << error.str() << endl;
    throw std::runtime_error(error.str());
  }
  qDebug() << "Opening transaction: " << QSqlDatabase::database().transaction();
//   qDebug() << "Cleanup database";
//   QSqlQuery removeDenominations;
//   removeDenominations.prepare("delete from denominations  where catalogue = :catalogue");
//   removeDenominations.bindValue(":catalogue", catalogue);
//   bool queryOk = removeDenominations.exec();
//   int affectedRows = removeDenominations.numRowsAffected();
//   if(!queryOk) {
//     qDebug() << __PRETTY_FUNCTION__ << ": Last error: " << removeDenominations.lastError().text();
//     throw std::runtime_error("Error removing denominations");
//   }
//   qDebug() << "denominations removal: " << queryOk << ", rowsAffected=" << affectedRows;
//   QSqlQuery removeOrphanObjects("delete from \"objects\" where id in (select \"objects\".id from \"objects\" left join denominations on \"objects\".id = denominations.objects_id where denominations.id is null)");
//   queryOk = removeOrphanObjects.exec();
//   affectedRows = removeOrphanObjects.numRowsAffected();
//   if(!queryOk) {
//     qDebug() << __PRETTY_FUNCTION__ << ": Last error: " << removeOrphanObjects.lastError().text();
//     throw std::runtime_error("Error removing denominations");
//   }
//   qDebug() << "orphan objects removal: " << queryOk << ", rowsAffected=" << affectedRows;
}


long long CatalogsImporter::findByName( const std::string &name )
{
  return findBy("SELECT \"objects\".id, * FROM \"objects\" \
    INNER JOIN denominations ON \"objects\".id = denominations.objects_id WHERE lower(denominations.name) LIKE '%'||:name||'%'  ",
                {{":name", QString::fromStdString(name).trimmed().toLower()}}
               );
}

long long int CatalogsImporter::findByCatalog( const std::string &catalogAndNumber )
{
//   qDebug() << __PRETTY_FUNCTION__ << ": catalogAndNumber=" << QString::fromStdString(catalogAndNumber);
  QStringList c = QString::fromStdString(catalogAndNumber).trimmed().split(" ");
  QString cat = c.first();
  return findByCatalog(cat.toStdString(), c.last().toStdString());
}


long long CatalogsImporter::findByCatalog( const std::string &catalog, const std::string &number )
{
  return findBy("SELECT objects_id from denominations WHERE \
    lower(denominations.catalogue)  = :catalogue AND \
    lower(denominations.\"number\" ) = :number \
    ", {
      {":catalogue", QString::fromStdString(catalog).trimmed().toLower()},
      {":number",  QString::fromStdString(number).trimmed().toLower()},
    });
}

long long CatalogsImporter::findBy(const std::string &query, const std::map<std::string,QVariant> &bindValues)
{
  QSqlQuery sqlQuery(db);
  sqlQuery.prepare( QString::fromStdString(query) );
  for(auto k: bindValues)
    sqlQuery.bindValue(QString::fromStdString(k.first), k.second);
  if(!sqlQuery.exec()) {
    dumpQuery( sqlQuery );
    return -1;
  }
  if(!sqlQuery.next())
    return -1;
  return sqlQuery.value(0).toLongLong();
}


long long CatalogsImporter::insertDenomination(std::string catalogueNumber, const std::string &name, const std::string &comment, long long objectId, int searchMode, const std::string &other_catalogues )
{
//   std::cerr << __PRETTY_FUNCTION__  << ", number= " << catalogueNumber << ", name=" << name << ", comment=" << comment << ", object_id= " << objectId << ", other_catalogues=" << other_catalogues << std::endl;
  QSqlQuery query(db);
  QString qCatNum = QString::fromStdString(catalogueNumber).trimmed();
  if(!other_catalogues.empty()) {
    query.prepare("INSERT INTO denominations(\"catalogue\", \"number\", \"name\", \"comment\", objects_id, search_mode, other_catalogues) \
    VALUES( :catalogue , :number , :name , :comment , :objectid, :search_mode , :othercatalogues )");
    query.bindValue(":othercatalogues", QString::fromStdString(other_catalogues).trimmed());
  } else {
    query.prepare("INSERT INTO denominations(\"catalogue\", \"number\", \"name\", \"comment\", objects_id, search_mode) \
    VALUES(:catalogue , :number , :name , :comment , :objectid , :search_mode)");
  }
  query.bindValue(":catalogue", catalogue);
  query.bindValue(":number", qCatNum);
  query.bindValue(":name", QString::fromStdString(name).trimmed());
  query.bindValue(":comment", QString::fromStdString(comment).trimmed());
  query.bindValue(":objectid", objectId);
  query.bindValue(":search_mode", searchMode);
  if(!query.exec()) {
    dumpQuery(query);
    return -1;
  }
  return lastInsertId(query, "SELECT id from denominations WHERE catalogue = :catalogue AND number = :number", {{":catalogue", catalogue}, {":number", qCatNum}});
}

long long CatalogsImporter::insertObject( const std::string &objectId, double rightAscension, double declination, double magnitude, double angularSize, int type )
{
//   std::cerr << __PRETTY_FUNCTION__ << ": object_id=" << objectId << ", ra= " << rightAscension<< ", dec=" << dec << ", magnitude=" << magnitude << ", angular_size= " << type<< ", type=" << type << std::endl;
  QSqlQuery query(db);
  QString qObjectId = QString::fromStdString(objectId).trimmed();
  query.prepare("INSERT INTO objects(object_id, \"ra\", \"dec\", magnitude, angular_size, type) \
    VALUES(:object_id, :ra, :dec, :magnitude, :angular_size, :type)");
  query.bindValue(":object_id", qObjectId);
  query.bindValue(":ra", rightAscension);
  query.bindValue(":dec", declination);
  query.bindValue(":magnitude", magnitude);
  query.bindValue(":angular_size", angularSize);
  query.bindValue(":type", type);
  if(!query.exec()) {
    dumpQuery(query);
    return -1;
  }
  return lastInsertId(query, "SELECT id from \"objects\" WHERE object_id = :objectid", {{":objectid", qObjectId}});
}

long long int CatalogsImporter::lastInsertId( QSqlQuery &query, const QString &selectQuery, std::map<QString,QVariant> bindValues )
{
//   qDebug() << __PRETTY_FUNCTION__;
  auto qLastInsertId = query.lastInsertId();
  if(qLastInsertId.toLongLong() > 0)
    return qLastInsertId.toLongLong();
  QSqlQuery getValue(db);
  getValue.prepare(selectQuery);
  for(auto v: bindValues)
    getValue.bindValue(v.first, v.second);
  if(! getValue.exec() || ! getValue.next() ) {
    dumpQuery(query);
    return -1;
  }
  return getValue.value(0).toLongLong();
}


#endif
