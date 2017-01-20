from skyplanner.models.db import db
from skyplanner.models.user import User
import sqlalchemy.exc
from itsdangerous import (TimedJSONWebSignatureSerializer
                          as Serializer, BadSignature, SignatureExpired)

class UsersController:
    def __init__(self, app):
        self.app = app
        self.logger = app.logger

    def login(self, data):
        user = User.query.filter_by(username=data['username']).first()
        if not user or not user.verify_password(data['password']):
            return {'result': 'wrong_user_or_password'}
        return {'result': 'ok', 'user': user.to_map(), 'token': self.auth_token(user)}

    def create(self, data):
        try:
            user = User(username = data['username'], password = data['password'])
            db.session.add(user)
            db.session.commit()
            return {'result': 'ok', 'user': user.to_map()}
        except sqlalchemy.exc.IntegrityError as e:
            self.logger.info(e)
            return {'result': 'error', 'error_type': 'username_existing'}
        except sqlalchemy.exc.SQLAlchemyError as e:
            self.logger.info(e)
            return {'result': 'error', 'error_type': 'unknown'}

    def auth_token(self, user, expiration = 600):
        s = Serializer(self.app.config['SECRET_KEY'], expires_in = expiration)
        return s.dumps({'id': user.id, 'username': user.username}).decode('utf-8')

    def verify_token(self, token):
        s = Serializer(self.app.config['SECRET_KEY'])
        try:
            data = s.loads(bytes(token, 'utf-8'))
        except SignatureExpired:
            return None # TODO: custom exception
        except BadSignature:
            return None # TODO: custom exception
        user = User.query.get(data['id'])
        return user
