from skyplanner.models.db import db
from skyplanner.models.user import User
from skyplanner.result_helpers import *
import sqlalchemy.exc
from itsdangerous import (TimedJSONWebSignatureSerializer
                          as Serializer, BadSignature, SignatureExpired)


class UsersController:
    class UserOrPasswordError(RuntimeError):
        pass
    class TokenExpiredError(RuntimeError):
        pass
    class BadTokenError(RuntimeError):
        pass
    class UsernameExistingError(RuntimeError):
        pass
    class UserRegistrationError(RuntimeError):
        pass

    def __init__(self, app):
        self.app = app
        self.logger = app.logger

    def login(self, data):
        user = User.query.filter_by(username=data['username']).first()
        if not user or not user.verify_password(data['password']):
            raise UsersController.UserOrPasswordError()
        return result_ok(user=user.to_map(), token=self.auth_token(user))

    def create(self, data):
        try:
            user = User(username = data['username'], password = data['password'])
            db.session.add(user)
            db.session.commit()
            return result_ok(user = user.to_map())
        except sqlalchemy.exc.IntegrityError as e:
            self.logger.info(e)
            return result_error(reason='username_existing')
        except sqlalchemy.exc.SQLAlchemyError as e:
            self.logger.info(e)
            return result_error(reason='unknown')

    def auth_token(self, user, expiration = 600):
        s = Serializer(self.app.config['SECRET_KEY'], expires_in = expiration)
        return s.dumps({'id': user.id, 'username': user.username}).decode('utf-8')

    def verify_token(self, token):
        s = Serializer(self.app.config['SECRET_KEY'])
        try:
            data = s.loads(bytes(token, 'utf-8'))
        except SignatureExpired:
            raise TokenExpiredError()
        except BadSignature:
            raise BadTokenError()
        user = User.query.get(data['id'])
        return user
