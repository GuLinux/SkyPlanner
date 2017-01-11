import ephem

class SkySector:
    VERSION = 1
    DEGREES_STEP = 5
    __sectors = None

    def __init__(self, ar, dec, index):
        self.ar = ar
        self.dec = dec
        self.index = index
        self.center = {'ar': ar[0] + (ar[1] - ar[0])/2, 'dec': dec[0] + (dec[1] - dec[0])/2}


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
                for ar in range(0, 360, SkySector.DEGREES_STEP):
                    ar_range = ephem.hours(ephem.degree * ar), ephem.hours(ephem.degree * (ar + SkySector.DEGREES_STEP))
                    SkySector.__sectors.append(SkySector(ar_range, dec_range, index))
                    index += 1
        return SkySector.__sectors

