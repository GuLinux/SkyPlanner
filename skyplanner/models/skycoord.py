from skyplanner.models.angle import Angle
class SkyCoord:
    def __init__(self, ra, dec):
        self.ra = ra if type(ra) is Angle else Angle(ra)
        self.dec = dec if type(dec) is Angle else Angle(dec)

    def __str__(self):
        return 'ra: {0}, dec: {1}'.format(self.ra, self.dec)

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return hash((self.ra, self.dec))

    def __eq__(self, other):
        return self.ra == other.ra and self.dec == other.dec
