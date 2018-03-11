import json
from enum import Enum

import dap.models as models
import requests

from . import DAPServer


def jsonDefault(o):
    if isinstance(o, Enum):
        return o.value
    return o.__dict__


class Service:
    """description of class"""

    def __init__(self, dap_server_or_url='http://dap.geosoft.com/rest/'):
        if type(dap_server_or_url) is DAPServer:
            self.dap_server = dap_server_or_url
        else:
            self.dap_server = DAPServer(dap_server_or_url)

    def default_resolution(self, parameters=models.DefaultExtractionResolutionParameters()):
        service_url = self.dap_server.url + 'service/extraction/default_resolution?key=test';

        headers = {'Content-Type': 'application/json', 'Accept': 'application/json'}
        response = requests.post(service_url, data=json.dumps(parameters, default=jsonDefault), headers=headers)
        if (response.ok):
            resolution = json.loads(response.content)
            return resolution
        else:
            response.raise_for_status()
