#ifndef CATALOGS_PARSING_DB_HELPER
#define CATALOGS_PARSING_DB_HELPER

#include <QCoreApplication>
#include <QStringList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>

class CatalogsImporter {
public:
  CatalogsImporter(int argc, char **argv);
  CatalogsImporter(QCoreApplication &app);
  ~CatalogsImporter();
  long long insertObject(const std::string &objectId, double rightAscension, double declination, double magnitude, double angularSize, int type);
  long long insertDenomination(const std::string &catalogue, int catalogueNumber, const std::string &name, const std::string &comment, long long objectId);
  long long findByName(const std::string &name);
  long long findByCatalog(const std::string &catalog, int number);
  long long findByCatalog(const std::string &catalogAndNumber);
private:
  void init(QStringList arguments);
  QSqlDatabase db;
};

CatalogsImporter::CatalogsImporter( int argc, char **argv )
{
  QCoreApplication app(argc, argv);
  init(app.arguments());
}


CatalogsImporter::CatalogsImporter( QCoreApplication &app )
{
  init(app.arguments());
}

CatalogsImporter::~CatalogsImporter()
{
  qDebug() << __PRETTY_FUNCTION__ << ": Commit: " << QSqlDatabase::database().commit();
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
}


long long CatalogsImporter::findByName( const std::string &name )
{
  qDebug() << __PRETTY_FUNCTION__ << ": name=" << QString::fromStdString(name);
  QSqlQuery query(db);
  query.prepare("SELECT \"objects\".id, * FROM \"objects\" \
    INNER JOIN denominations ON \"objects\".id = denominations.objects_id WHERE lower(denominations.name) LIKE '%'||:name||'%'  ");
  query.bindValue("name", QString::fromStdString(name).trimmed().toLower());
  if(!query.exec() || ! query.next()) return -1;
  return query.value(0).toLongLong();
}

long long int CatalogsImporter::findByCatalog( const std::string &catalogAndNumber )
{
  qDebug() << __PRETTY_FUNCTION__ << ": catalogAndNumber=" << QString::fromStdString(catalogAndNumber);
  QStringList c = QString::fromStdString(catalogAndNumber).trimmed().split(" ");
  QString cat = c.first();
  int number = c.last().toInt();
  return findByCatalog(cat.toStdString(), number);
}


long long CatalogsImporter::findByCatalog( const std::string &catalog, int number )
{
  qDebug() << __PRETTY_FUNCTION__ << ": catalog=" << QString::fromStdString(catalog) << ", number=" << number;
  QSqlQuery query(db);
  query.prepare("SELECT \"objects\".id, * FROM \"objects\" \
    INNER JOIN denominations ON \"objects\".id = denominations.objects_id WHERE lower(denominations.catalogue)  = :catalogue \
    AND denominations.\"number\" = :number \
    ");
  query.bindValue("catalogue", QString::fromStdString(catalog).trimmed().toLower());
  query.bindValue("number", number);
  if(!query.exec() || ! query.next()) {
    qDebug() << "Last error: " << query.lastError().text();
    return -1;
  }
  return query.value(0).toLongLong();
}

long long CatalogsImporter::insertDenomination( const std::string &catalogue, int catalogueNumber, const std::string &name, const std::string &comment, long long objectId )
{
  QSqlQuery query(db);
  query.prepare("INSERT INTO denominations(\"catalogue\", \"number\", \"name\", \"comment\", objects_id) \
    VALUES(:catalogue, :number, :name, :comment, :object_id )");
  query.bindValue("catalogue", QString::fromStdString(catalogue).trimmed());
  query.bindValue("number", catalogueNumber);
  query.bindValue("name", QString::fromStdString(name).trimmed());
  query.bindValue("comment", QString::fromStdString(comment).trimmed());
  query.bindValue("object_id", objectId);
  if(!query.exec()) return -1;
  return query.lastInsertId().toLongLong();
}

long long CatalogsImporter::insertObject( const std::string &objectId, double rightAscension, double declination, double magnitude, double angularSize, int type )
{
  QSqlQuery query(db);
  query.prepare("INSERT INTO objects(object_id, \"ra\", \"dec\", magnitude, angular_size, type) \
    VALUES(:object_id, :ra, :dec, :magnitude, :angular_size, :type)");
  query.bindValue("object_id", QString::fromStdString(objectId).trimmed());
  query.bindValue("ra", rightAscension);
  query.bindValue("dec", declination);
  query.bindValue("magnitude", magnitude);
  query.bindValue("angular_size", angularSize);
  query.bindValue("type", type);
  if(!query.exec()) return -1;
  return query.lastInsertId().toLongLong();
}

#endif
