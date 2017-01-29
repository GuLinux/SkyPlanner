from skyplanner.models.db import db
from skyplanner.models.user import User
from skyplanner.result_helpers import *
import sqlalchemy.exc
from itsdangerous import (TimedJSONWebSignatureSerializer
                          as Serializer, BadSignature, SignatureExpired)
from skyplanner.errors import SkyPlannerError

class UsersController:
    class Error(RuntimeError):
        def __init__(self, reason = 'unknown'):
            self.reason = reason


    def __init__(self, app):
        self.app = app
        self.logger = app.logger

    def login(self, data):
        user = None
        try:
            user = User.query.filter_by(username=data['username']).one()
        except sqlalchemy.orm.exc.NoResultFound:
            pass
        if not user or not user.verify_password(data['password']):
            raise SkyPlannerError.Unauthorized({'error': 'wrong_user_or_password'})
        return result_ok(user=user.to_map(), token=self.auth_token(user))

    def create(self, data):
        if not 'username' in data or len(data['username']) < 4:
            raise SkyPlannerError.BadRequest({'error': 'username_too_short'})
        if not 'password' in data or len(data['password']) < 8:
            raise SkyPlannerError.BadRequest({'error': 'password_too_short'})
        try:
            user = User(username = data['username'], password = data['password'])
            db.session.add(user)
            db.session.commit()
            return result_ok(user = user.to_map())
        except sqlalchemy.exc.IntegrityError as e:
            raise SkyPlannerError.Conflict({'error': 'username_already_existing'})

    def auth_token(self, user, expiration = 86400): # todo: back to 600?
        s = Serializer(self.app.config['SECRET_KEY'], expires_in = expiration)
        return s.dumps({'id': user.id}).decode('utf-8')

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

class UserRegistrationError(UsersController.Error):
    def __init__(self, reason):
        UsersController.Error.__init__(self, reason if reason else 'generic_registration_error')

