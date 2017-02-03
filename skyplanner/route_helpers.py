from flask import json, request
from skyplanner.controllers.users import UsersController
from skyplanner.controllers.telescopes import TelescopesController

from skyplanner.result_helpers import result_ok, result_error
from functools import wraps
from skyplanner.errors import SkyPlannerError

app = None
controllers = dict()

def json_ok(**kwargs):
    return json.jsonify(result_ok(**kwargs))

def json_error(reason, **kwargs):
    return json.jsonify(result_error(reason, **kwargs))

def controller(func):
    @wraps(func)
    def wrapper():
        name = func.__name__
        if not name in controllers:
            controllers[name] = func()
        return controllers[name]
    return wrapper

@controller
def users_controller():
    return UsersController(app)

@controller
def telescopes_controller():
    return TelescopesController(app)


def get_user_from_token(request):
    user = None
    if request.args.get('auth'):
        user = users_controller().verify_token(request.args.get('auth'))
    if not user:
        raise SkyPlannerError.Unauthorized({'error': 'token_required'})
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

