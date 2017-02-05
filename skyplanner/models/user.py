from skyplanner.models import db
import bcrypt

class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String, nullable = False, unique = True)
    password_hash = db.Column(db.String, nullable = False)
    pupil = db.Column(db.Float(precision = 1), default = 6.5)
    telescopes = db.relationship('Telescope', backref='user', lazy='select')
    observations = db.relationship('Observation', backref='user', lazy='select')

    def __init__(self, username, password):
        self.username = username
        self.set_password(password)

    def verify_password(self, password):
        return bcrypt.checkpw(bytes(password, 'utf-8'), self.password_hash)

    def set_password(self, password):
        self.password_hash = bcrypt.hashpw(bytes(password, 'utf-8'), bcrypt.gensalt())

    def to_map(self):
        return {'username': self.username, 'id': self.id}

    def __str__(self):
        return 'id: {0}, username: {1}'.format(self.id, self.username)

    def __repr__(self):
        return self.__str__()
