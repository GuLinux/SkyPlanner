import unittest
from skyplanner.ephemeris.native.ephemeris import *
from astropy.time import Time
from astropy.coordinates import SkyCoord


class TestEphemeris(unittest.TestCase):
    def setUp(self):
        self.ephemeris = Ephemeris(51.507351, -0.127758)
        self.antares = SkyCoord('16h 29m 24.45970s', '-26° 25′ 55.2094″')
        self.vega = SkyCoord('18h 36m 56.33635s', '+38° 47′ 01.2802″')
        self.alpha_cen = SkyCoord('14h 39m 36.49400s', '-60° 50′ 02.3737″')
        self.day = Time('2017-01-14T00:00:00', 'iso')

    def test_rst_object_circumpolar(self):
        rst = self.ephemeris.rst(coordinates(self.vega.ra.degree, self.vega.dec.degree), self.day.jd, 0)
        self.assertEqual(RiseTransitSet.Type.CircumPolar, rst.type)

    def test_rst_object_rising(self):
        rst = self.ephemeris.rst(coordinates(self.antares.ra.degree, self.antares.dec.degree), self.day.jd, 0)
        self.assertEqual(RiseTransitSet.Type.Rises, rst.type)
        self.assertEqual('2017-01-14 05:29:10.472', Time(rst.rise, format='jd').iso)
        self.assertEqual('2017-01-14 08:53:51.253', Time(rst.transit, format='jd').iso)
        self.assertEqual('2017-01-14 12:18:32.373', Time(rst.set, format='jd').iso)

    def test_rst_object_never_rising(self):
        rst = self.ephemeris.rst(coordinates(self.alpha_cen.ra.degree, self.alpha_cen.dec.degree), self.day.jd, 0)
        self.assertEqual(RiseTransitSet.Type.NeverRises, rst.type)

    def test_rst_list(self):
        objects_coordinates = [coordinates(x.ra.degree, x.dec.degree) for x in [self.vega, self.alpha_cen, self.antares]]
        rst = self.ephemeris.rst_list(objects_coordinates, self.day.jd, 0)
        self.assertEqual(RiseTransitSet.Type.CircumPolar, rst[0].type)
        self.assertEqual(RiseTransitSet.Type.NeverRises, rst[1].type)
        self.assertEqual(RiseTransitSet.Type.Rises, rst[2].type)


