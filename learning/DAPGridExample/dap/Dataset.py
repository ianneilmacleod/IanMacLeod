import json
from enum import Enum

import dap.models as models
import requests

from . import DAPServer

import os

def jsonDefault(o):
    if isinstance(o, Enum):
        return o.value
    return o.__dict__


def decodeObject(o):
    if 'CoordinateSystem' in o:
        b = models.BoundingBox()
        b.__dict__.update(o)
        return b


class Dataset:
    """description of class"""

    def __init__(self, dap_server_or_url='http://dap.geosoft.com/rest/'):
        if isinstance(dap_server_or_url, DAPServer):
            self.dap_server = dap_server_or_url
        else:
            self.dap_server = DAPServer(dap_server_or_url)

    def grid_properties(self, dataset):
        grid_properties_url = self.dap_server.url + 'dataset/properties/grid/' + dataset.Id + '?key=test'
        headers = {'Accept': 'application/json'}
        response = requests.get(grid_properties_url, headers=headers)
        if (response.ok):
            data = json.loads(response.content)
            props = models.GridProperties()
            props.__dict__.update(data)
            return props
        else:
            response.raise_for_status()

    def extents(self, dataset):
        extents_url = self.dap_server.url + 'dataset/extents/' + dataset.Id + '?key=test'
        headers = {'Accept': 'application/json'}
        response = requests.get(extents_url, headers=headers)
        if (response.ok):
            data = json.loads(response.content, object_hook=decodeObject)
            return data
        else:
            response.raise_for_status()

    def metadata(self, dataset):
        metadata_url = self.dap_server.url + 'dataset/metadata/' + dataset.Id + '?key=test'
        response = requests.get(metadata_url)
        if (response.ok):
            return response.content
        else:
            response.raise_for_status()

    def draw(self, dataset):
        render_url = self.dap_server.url + 'dataset/draw/xml/' + dataset.Id + '?key=test'
        draw_request = models.DrawRequest()

        width = dataset.Extents.MaxX - dataset.Extents.MinX
        height = dataset.Extents.MaxY - dataset.Extents.MinY
        if (width == 0 or height == 0):
            raise ValueError("height or width is 0")

        if (height > width):
            draw_request.Width = int(draw_request.Height * width / height)
        else:
            draw_request.Height = int(draw_request.Width * height / width)
        draw_request.BoundingBox = dataset.Extents

        headers = {'Content-Type': 'application/json'}
        response = requests.post(render_url, data=json.dumps(draw_request, default=jsonDefault), headers=headers)
        if (response.ok):
            return response.content
        else:
            response.raise_for_status()

    def extract_resolution(self, dataset):
        url = self.dap_server.url + 'dataset/extract/resolution/' + dataset.Id + '?key=test'
        headers = {'Content-Type': 'application/json', 'Accept': 'application/json'}
        response = requests.post(url, data=json.dumps(dataset.Extents, default=jsonDefault), headers=headers)
        if (response.ok):
            data = json.loads(response.content)
            props = models.ExtractResolutions()
            props.__dict__.update(data)
            return props
        else:
            response.raise_for_status()

    def extract_grid(self, id, parameters):
        url = self.dap_server.url + 'dataset/extract/grid/' + id + '?key=test'
        headers = {'Content-Type': 'application/json', 'Accept': 'application/json'}
        response = requests.post(url, data=json.dumps(parameters, default=jsonDefault), headers=headers)
        if (response.ok):
            data = json.loads(response.content)
            return data
        else:
            response.raise_for_status()

    def extract_progress(self, key):
        url = self.dap_server.url + 'dataset/extract/progress/' + key + '?key=test'
        headers = {'Accept': 'application/json'}
        response = requests.get(url, headers=headers)
        if (response.ok):
            data = json.loads(response.content)
            props = models.ExtractProgress()
            props.__dict__.update(data)
            props.Stage = models.ExtractProgressStatus(props.Stage)
            return props
        else:
            response.raise_for_status()

    def extract_download_information(self, key):
        url = self.dap_server.url + 'dataset/extract/describe/' + key + '?key=test'
        headers = {'Accept': 'application/json'}
        response = requests.get(url, headers=headers)
        if (response.ok):
            data = json.loads(response.content)
            props = models.ExtractDownloadInformation()
            props.__dict__.update(data)
            return props
        else:
            response.raise_for_status()

    def extract_download(self, key, download_information, zip_file='dap_grid.zip', progress=None):
        url = self.dap_server.url + 'stream/dataset/extract/block/'
        parameter = '?key=test'

        with open(zip_file, 'wb') as out:  ## Open temporary file as bytes
            for index in range(download_information.NumberOfBlocks):
                if progress:
                    progress('Fetching {} of {}: {}'.format(index + 1, download_information.NumberOfBlocks, zip_file))
                extract_url = url + key + "/" + str(index) + parameter
                response = requests.get(extract_url)
                if (response.ok):
                    out.write(response.content)
                else:
                    response.raise_for_status()

        return os.path.normpath(zip_file)
