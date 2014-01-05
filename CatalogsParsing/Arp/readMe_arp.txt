 
Byte-by-byte Description of: arplist.dat
------------------------------------------------------------------------
 bytes   format   units  label    explanations
------------------------------------------------------------------------
 01-003  I3       ---    Arp      Arp number from original catalog
 05-020  A16      ---    Name     Common name of galaxy
 21-024  F4.1     mag    VT      *?Total V-magnitude
    025  A1       ---    u_VT     [?] Uncertain magnitude = ?
 27-030  F4.1   arcmin   dim1     ?Larger dimension of galaxy
    031  A1       ---    ---      [X]
 32-035  F4.1   arcmin   dim2     ?Smaller dimension (if differs from dim1)
    037  A1       ---    u_dim2   [?] Uncertain size = ?
 39-052  A14      ---    MType   *Morphological Type
 53-055  I3       ---    Uchart  *?Uranometria chart number
 58-059  I2       h      RAh      Right Ascension J2000 (hours)
 61-062  I2       min    RAm      Right Ascension J2000 (minutes)
 64-067  F4.1     s      RAs      Right Ascension J2000 (seconds)
    069  A1       ---    DE-      Declination J2000 (sign)
 70-071  I2       deg    DEd      Declination J2000 (degrees)
 73-074  I2       arcmin DEm      Declination J2000 (minutes)
 76-077  I2       arcsec DEs      Declination J2000 (seconds)
--------------------------------------------------------------------------------
