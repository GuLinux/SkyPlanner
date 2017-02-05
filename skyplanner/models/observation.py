from skyplanner.models.db import db

class Observation(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String)
    lat = db.Column(db.Float(precision=10), nullable = False)
    lng = db.Column(db.Float(precision=10), nullable = False)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))

    def __init__(self, name, lat, lng, user):
        self.name = name
        self.lat = lat
        self.lng = lng
        self.user_id = user.id

    def to_map(self):
        return {'id': self.id, 'name': self.name, 'lat': self.lat, 'lng': self.lng}

    def __str__(self):
        return 'id: {0}, name: {1}, lat: {2}, lng: {3}, user_id: {4}' \
                .format(self.id, self.name, self.lat, self.lng, self.user_id)

    def __repr__(self):
        return self.__str__()

