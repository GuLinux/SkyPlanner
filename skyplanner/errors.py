from skyplanner.result_helpers import result_ok, result_error
from flask import json

class SkyPlannerError(RuntimeError):
    SERVER_ERROR = 500
    NOT_FOUND = 404
    UNAUTHORIZED = 401
    FORBIDDEN = 403
    BAD_REQUEST = 400
    CONFLICT = 409

    def __init__(self, message = 'generic_error', status = SERVER_ERROR, payload = {}):
        super().__init__(message)
        self.message = message
        self.status = status
        self.payload = payload

    def BadRequest(message = 'bad_request', payload = {}):
        return SkyPlannerError(message=message, status=SkyPlannerError.BAD_REQUEST, payload = payload)

    def NotFound(message='not_found', payload = {}):
        return SkyPlannerError(message=message, status=SkyPlannerError.NOT_FOUND, payload = payload)

    def Unauthorized(message='unauthorized', payload = {}):
        return SkyPlannerError(message=message, status=SkyPlannerError.UNAUTHORIZED, payload = payload)

    def Conflict(message='conflict', payload = {}):
        return SkyPlannerError(message=message, status=SkyPlannerError.CONFLICT, payload = payload)

    def response(self):
        return json.jsonify(result_error(self.message, data=self.payload)), self.status
