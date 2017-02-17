from skyplanner.errors import SkyPlannerError
from skyplanner.models import Observation, db
from skyplanner.result_helpers import result_ok, result_error
import sqlalchemy

class ObservationsController:
    def __init__(self, app):
        self.app = app

    def all(self, user):
        return [t.to_map() for t in user.observations]

    def create(self,user, data):
        observation = Observation(data['name'], data['lat'], data['lng'], user)
        db.session.add(observation)
        db.session.commit()

    def remove(self, user, id):
        result = Observation.query.filter_by(id = id, user_id = user.id).delete()
        db.session.commit()
        if result == 0:
            raise SkyPlannerError.NotFound('observation_not_found')
        return result_ok()

    def update(self, user, id, data):
        observation = Observation.query.filter_by(id = id, user_id = user.id).one()
        observation.update(data)
        db.session.commit()
        return result_ok()


