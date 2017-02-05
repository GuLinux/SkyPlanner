from skyplanner.models import SkyCoord, Angle
from skyplanner.ephemeris.native.ephemeris import coordinates

from bisect import bisect_right

class SkySector:
    VERSION = 1
    DEGREES_STEP = 5
    __sectors = None
    __sectors_ra = []
    __sectors_dec = []

    def __init__(self, start, end, index):
        self.start = start
        self.end = end
        self.index = index
        self.center = SkyCoord( start.ra + (end.ra - start.ra)/2, start.dec + (end.dec - start.dec)/2)

    def belongs_to(self, coords):
        return self.start.ra < coords.ra < self.end.ra and self.start.dec < coords.dec < self.end.dec

    def __str__(self):
        return 'AR: {0} to {1}; DEC: {2} to {3}'.format(self.start.ra, self.end.ra, self.start.dec, self.end.dec)

    def __repr__(self):
        return 'index: {0}, coords: {1}'.format(self.index, self.__str__())

    def all():
        if not SkySector.__sectors:
            SkySector.__sectors = list()
            index = 0
            for dec in range(-90, 90, SkySector.DEGREES_STEP):
                SkySector.__sectors_dec.append((dec, index))
                dec_end = dec + SkySector.DEGREES_STEP

                for ra in range(0, 360, SkySector.DEGREES_STEP):
                    SkySector.__sectors_ra.append((ra, index))
                    ra_end = ra + SkySector.DEGREES_STEP

                    SkySector.__sectors.append(SkySector( SkyCoord(ra, dec), SkyCoord(ra_end, dec_end) , index))
                    index += 1
        return SkySector.__sectors

    def find_sector(coords):
        all_sectors = SkySector.all()
        sectors_dec = [x[0] for x in SkySector.__sectors_dec]
        index_dec =  SkySector.__sectors_dec[bisect_right(sectors_dec, coords.dec.degrees)-1][1]

        ra_sectors_for_dec = [x for x in SkySector.__sectors_ra if x[1] >= index_dec and x[1] < index_dec + 360/SkySector.DEGREES_STEP]
        ra_sectors = [x[0] for x in ra_sectors_for_dec]
        index = ra_sectors_for_dec[bisect_right(ra_sectors, coords.ra.degrees)-1][1]
        return all_sectors[index]

    def rise_transit_set(ephemeris, jd, horizon = Angle(0)):
        sectors_coords = [coordinates(x.center.ra.degrees, x.center.dec.degrees) for x in SkySector.all() ]
#        return ephemeris.rst_parallel(sectors_coords, jd, horizon.degrees)
        return ephemeris.rst_list(sectors_coords, jd, horizon.degrees)



