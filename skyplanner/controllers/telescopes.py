from skyplanner.errors import SkyPlannerError
from skyplanner.models import Telescope
from skyplanner.models import db
from skyplanner.result_helpers import result_ok, result_error
import sqlalchemy

class TelescopesController:
    def __init__(self, app):
        self.app = app

    def all(self, user):
        return [t.to_map() for t in user.telescopes]

    def add(self,user, data):
        if not set(['name', 'focal_length', 'diameter']) < data.keys():
            raise SkyPlannerError.BadRequest('not_enough_parameters')
        telescope = Telescope(data['name'], data['focal_length'], data['diameter'], user)
        self.validate(telescope)
        db.session.add(telescope)
        db.session.commit()
        return telescope.to_map()

    def remove(self, user, telescope_id):
        result = Telescope.query.filter_by(id = telescope_id, user_id = user.id).delete()
        db.session.commit()
        if result == 0:
            raise SkyPlannerError.NotFound('telescope_not_found')
        return result_ok()

    def edit(self, user, telescope_id, data):
        telescope = None
        try:
            telescope = Telescope.query.filter_by(id = telescope_id, user_id = user.id).one()
        except sqlalchemy.orm.exc.NoResultFound:
            raise SkyPlannerError.NotFound('telescope id {0} for user {1} not found'.format(telescope_id, user.id))
        telescope.update(data)
        self.validate(telescope)
        db.session.commit()
        return telescope.to_map()


    def validate(self, telescope):
        validation = telescope.validate()
        if not validation[0]:
            raise SkyPlannerError.BadRequest('invalid_telescope_data', {'errors': validation[1]} )
