from flask_login import UserMixin
from skyplanner.models.db import db
import bcrypt

class User(db.Model, UserMixin):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String, nullable = False)
    password_hash = db.Column(db.String, nullable = False)

    def __init__(self, username, password):
        self.username = username
        self.password_hash = bcrypt.hashpw(bytes(password, 'utf-8'), bcrypt.gensalt())

    def verify_password(self, password):
        return bcrypt.checkpw(password, self.password_hash)
