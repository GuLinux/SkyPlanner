from flask import json, request
from skyplanner.controllers.users import UsersController
from skyplanner.result_helpers import result_ok, result_error
from functools import wraps
from skyplanner.errors import SkyPlannerError

app = None
controllers = dict()

def json_ok(**kwargs):
    return json.jsonify(result_ok(**kwargs))

def json_error(reason, **kwargs):
    return json.jsonify(result_error(reason, **kwargs))


def auth_url(func):
    @wraps(func)
    def func_wrapper(*args, **kwargs):
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
    return func_wrapper

def users_controller():
    def create():
        return UsersController(app)
    return get_controller('users', create)

def get_controller(name, factory):
    if not name in controllers:
        controllers[name] = factory()
    return controllers[name]


def get_user_from_token(request):
    user = None
    reason = 'token_required'
    try:
        if request.args.get('auth'):
            user = users_controller().verify_token(request.args.get('auth'))
    except UsersController.Error as e:
        reason = e.__class__.__name__
    if not user:
        raise SkyPlannerError.Unauthorized({'error': reason})
    return user


def skyplanner_api(url, auth_required = False, **kwargs):
    def api_decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            try:
                if auth_required:
                    kwargs['user'] = get_user_from_token(request)
                return func(*args, **kwargs)
            except SkyPlannerError as e:
                return e.response()
            except:
                app.logger.warning('Unexpected error on %s', func.__name__, exc_info = True)
                return json_error(reason='Unexpected server error'), 500
        app.add_url_rule(url, func.__name__, wrapper, **kwargs)
        return wrapper
    return api_decorator

