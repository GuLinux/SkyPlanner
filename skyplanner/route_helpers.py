from flask import json, request
from skyplanner.controllers.users import UsersController
from skyplanner.result_helpers import result_ok, result_error

app = None
controllers = dict()

def json_ok(**kwargs):
    return json.jsonify(result_ok(**kwargs))

def json_error(reason, **kwargs):
    return json.jsonify(result_error(reason, **kwargs))


def auth_url(func):
    def wrapper(*args, **kwargs):
        user = None
        try:
            if request.args.get('auth'):
                user = users_controller().verify_token(request.args.get('auth'))
            if not user:
                return json_error(reason='auth_required'), 401
            kwargs['user'] = user
            return func(*args, **kwargs)
        except UsersController.Error as e:
            return json_error(reason=e.reason), 401 
    return wrapper

def users_controller():
    def create():
        return UsersController(app)
    return get_controller('users', create)

def get_controller(name, factory):
    if not name in controllers:
        controllers[name] = factory()
    return controllers[name]

