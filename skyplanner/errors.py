from skyplanner.result_helpers import result_ok, result_error
from flask import json

class SkyPlannerError(RuntimeError):
    SERVER_ERROR = 500
    NOT_FOUND = 404
    UNAUTHORIZED = 401
    FORBIDDEN = 403
    BAD_REQUEST = 400

    def __init__(self, message = 'generic_error', status = SERVER_ERROR, payload = {}):
        super().__init__(message)
        self.message = message
        self.status = status
        self.payload = payload

    def BadRequest(payload = {}):
        return SkyPlannerError(message='bad_request', status=SkyPlannerError.BAD_REQUEST, payload = payload)

    def NotFound(payload = {}):
        return SkyPlannerError(message='not_found', status=SkyPlannerError.NOT_FOUND, payload = payload)

    def Unauthorized(payload = {}):
        return SkyPlannerError(message='unauthorized', status=SkyPlannerError.UNAUTHORIZED, payload = payload)

    def response(self):
        return json.jsonify(result_error(self.message, data=self.payload)), self.status
