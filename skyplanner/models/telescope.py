from skyplanner.models.db import db

class Telescope(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String)
    focal_length = db.Column(db.Integer)
    diameter = db.Column(db.Integer)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))

    def __init__(self, name, focal_length, diameter, user):
        self.name = name
        self.focal_length = focal_length
        self.diameter = diameter
        self.user_id = user.id

    def __str__(self):
        return 'id: {0}, name: {1}, focal_length: {2}, diameter: {3}, user_id: {4}' \
                .format(self.id, self.name, self.focal_length, self.diameter, self.user_id)

    def __repr__(self):
        return self.__str__()
