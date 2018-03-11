import json

import dap.models as models
import requests

from . import DAPServer as gxds


def jsonDefault(o):
    return o.__dict__


def decodeObject(o):
    if 'CoordinateSystem' in o:
        b = models.BoundingBox()
        b.__dict__.update(o)
        return b
    else:
        d = models.Dataset()
        d.__dict__.update(o)
        d.Type = models.DatasetType(d.Type)
        return d


class Catalog:
    """description of class"""

    def __init__(self, dap_server_or_url='http://dap.geosoft.com/rest/'):
        if isinstance(dap_server_or_url, gxds.DAPServer):
            self.dap_server = dap_server_or_url
        else:
            self.dap_server = gxds.DAPServer(dap_server_or_url)

    def search(self, search_parameters=models.SearchParameters()):
        search_url = self.dap_server.url + 'catalog/search?key=test'

        headers = {'Content-Type': 'application/json', 'Accept': 'application/json'}
        response = requests.post(search_url, data=json.dumps(search_parameters, default=jsonDefault), headers=headers)
        if (response.ok):
            data = json.loads(response.content, object_hook=decodeObject)
            return data
        else:
            response.raise_for_status()
