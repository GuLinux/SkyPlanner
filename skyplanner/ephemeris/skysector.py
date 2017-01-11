import ephem
from bisect import bisect_right
class SkySector:
    VERSION = 1
    DEGREES_STEP = 5
    __sectors = None
    __sectors_ar = []
    __sectors_dec = []

    def __init__(self, ar, dec, index):
        self.ar = ar
        self.dec = dec
        self.index = index
        self.center = {'ar': ar[0] + (ar[1] - ar[0])/2, 'dec': dec[0] + (dec[1] - dec[0])/2}
        self.body = ephem.FixedBody()
        self.body._ra = self.center['ar']
        self.body._dec = self.center['dec']

    def belongs_to(self, ar, dec):
        return self.ar[0] < ar < self.ar[1] and self.dec[0] < dec < self.dec[1]

    def __str__(self):
        return 'AR: {0} to {1}; DEC: {2} to {3}'.format(self.ar[0], self.ar[1], self.dec[0], self.dec[1])

    def __repr__(self):
        return 'index: {0}, coords: {1}'.format(self.index, self.__str__())

    def all():
        if not SkySector.__sectors:
            SkySector.__sectors = list()
            index = 0
            for dec in range(-90, 90, SkySector.DEGREES_STEP):
                dec_range = ephem.degrees(ephem.degree * dec), ephem.degrees(ephem.degree * (dec + SkySector.DEGREES_STEP))
                SkySector.__sectors_dec.append((dec_range[0], index))
                for ar in range(0, 360, SkySector.DEGREES_STEP):
                    ar_range = ephem.hours(ephem.degree * ar), ephem.hours(ephem.degree * (ar + SkySector.DEGREES_STEP))
                    SkySector.__sectors_ar.append((ar_range[0], index))
                    SkySector.__sectors.append(SkySector(ar_range, dec_range, index))
                    index += 1
        return SkySector.__sectors

    def find_sector(ar, dec):
        all_sectors = SkySector.all()
        sectors_dec = [x[0] for x in SkySector.__sectors_dec]
        index_dec =  SkySector.__sectors_dec[bisect_right(sectors_dec, dec)-1][1]

        ar_sectors_for_dec = [x for x in SkySector.__sectors_ar if x[1] >= index_dec and x[1] < index_dec + 360/SkySector.DEGREES_STEP]
        ar_sectors = [x[0] for x in ar_sectors_for_dec]
        index = ar_sectors_for_dec[bisect_right(ar_sectors, ar)-1][1]
        return all_sectors[index]

    def rise_transit_set(observer):
        rts = []
        for sector in SkySector.all():
            try:
                r = observer.next_rising(sector.body)
                t = observer.next_transit(sector.body)
                s = observer.next_setting(sector.body)
                rts.append((r, t, s))
            except ephem.NeverUpError:
                pass
            except ephem.AlwaysUpError:
                pass
        return rts
