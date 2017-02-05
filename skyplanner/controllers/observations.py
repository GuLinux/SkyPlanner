from skyplanner.errors import SkyPlannerError
from skyplanner.models.observation import Observation
from skyplanner.models.db import db
from skyplanner.result_helpers import result_ok, result_error
import sqlalchemy

class ObservationsController:
    def __init__(self, app):
        self.app = app

    def all(self, user):
        return [t.to_map() for t in user.observations]

    def add(self,user, data):
        pass
