"""How text coming back from SECONDO is decoded.

SECONDO has no encoding: a string is bytes. berlintest is Latin-1, anything
imported from a modern shapefile is UTF-8 (``dbimport`` copies DBF character
fields verbatim and never reads the codepage byte), and both live in the same
server. The bridge therefore recognises the encoding rather than assuming one;
these pin the rule, which is worth having on its own because no single fixture
database can exercise both halves of it.
"""
from __future__ import annotations

import secondo_native

decode = secondo_native.decode_server_text


def test_ascii_is_unambiguous():
    assert decode(b"Mehringdamm") == "Mehringdamm"


def test_utf8_shapefile_text():
    # `Bücherhalle Barmbek` as it sits in gis_osm_pois_free_1.dbf.
    assert decode(b"B\xc3\xbccherhalle") == "Bücherhalle"


def test_latin1_berlintest_text():
    # 0xfc alone is not a valid UTF-8 lead byte, so this can only be Latin-1.
    assert decode(b"Sch\xfctzenstra\xdfe") == "Schützenstraße"


def test_latin1_wins_when_utf8_would_be_invalid():
    """The pair 0xfc 0x62 is what berlintest's German text looks like, and is
    exactly what a UTF-8-only decoder used to raise on."""
    assert decode(b"f\xfcr") == "für"


def test_utf8_wins_when_the_bytes_are_valid_utf8():
    """The same bytes read as Latin-1 would give the mojibake this fixes --
    'BÃ¼cherhalle' -- so the order of the two attempts is the whole rule."""
    assert decode("Hogrevestraße".encode("utf-8")) == "Hogrevestraße"
    assert decode("Hogrevestraße".encode("utf-8")) != "HogrevestraÃ\x9fe"


def test_characters_outside_latin1_survive():
    """Recoding the database to Latin-1 would lose these; decoding cannot.
    Both are real values in the Hamburg extract."""
    assert decode("Meşhur Adıyaman".encode("utf-8")) == "Meşhur Adıyaman"
    assert decode("Hoppe’s".encode("utf-8")) == "Hoppe’s"


def test_no_byte_sequence_can_fail():
    """Latin-1 is the fallback precisely because it always succeeds: a truncated
    multi-byte sequence, or binary noise, must still come back as a str rather
    than raising in the middle of reading an answer."""
    assert decode(b"\xc3") == "Ã"
    assert isinstance(decode(bytes(range(256))), str)
    assert len(decode(bytes(range(256)))) == 256
