# ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# October 2019
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Secondo Messages
# secondomessages.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module Secondo Messages contains the start and end strings of the responses to an inquiry or a command from the
|sec| server.
"""

SECONDO_OK = '<SecondoOk/>'
SECONDO_INTRO_START = '<SecondoIntro>'
SECONDO_INTRO_END = '</SecondoIntro>'
SECONDO_ERROR_START = '<SecondoError>'
SECONDO_ERROR_END = '</SecondoError>'

SECONDO_CONNECT_START = '<Connect>\n'
SECONDO_CONNECT_END = '</Connect>\n'
SECONDO_DISCONNECT_END = '<Disconnect/>\n'

SECONDO_COMMAND_START = '<Secondo>\n'
SECONDO_COMMAND_END = '</Secondo>\n'

SECONDO_MESSAGE_START = '<Message>'
SECONDO_MESSAGE_END = '</Message>'

SECONDO_RESPONSE_START = '<SecondoResponse>'
SECONDO_RESPONSE_END = '</SecondoResponse>'

SECONDO_PROGRESS = 'progress'
