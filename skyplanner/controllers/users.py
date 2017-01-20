from skyplanner.models.db import db
from skyplanner.models.user import User
from skyplanner.result_helpers import *
import sqlalchemy.exc
from itsdangerous import (TimedJSONWebSignatureSerializer
                          as Serializer, BadSignature, SignatureExpired)


class UsersController:
    class Error(RuntimeError):
        def __init__(self, reason = 'unknown'):
            self.reason = reason


    def __init__(self, app):
        self.app = app
        self.logger = app.logger

    def login(self, data):
        user = User.query.filter_by(username=data['username']).first()
        if not user or not user.verify_password(data['password']):
            raise UserOrPasswordError()
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

class UserOrPasswordError(UsersController.Error):
    def __init__(self):
        UsersController.Error.__init__(self, reason='wrong_user_or_password')

class TokenExpiredError(UsersController.Error):
    def __init__(self):
        UsersController.Error.__init__(self, reason='token_expired')

class BadTokenError(UsersController.Error):
    def __init__(self):
        UsersController.Error.__init__(self, reason='bad_token')

class UsernameExistingError(UsersController.Error):
    def __init__(self):
        UsersController.Error.__init__(self, reason='username_already_existing')

class UserRegistrationError(UsersController.Error):
    def __init__(self):
        UsersController.Error.__init__(self, reason='generic_registration_error')

