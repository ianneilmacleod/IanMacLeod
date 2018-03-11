import time
from io import BytesIO

import dap as gxdap
import dap.models as gxdapmodels
import geosoft.gxpy.gx as gx
import geosoft.gxpy.utility as gxu
from PIL import Image


def find_dataset(list_data, dataset):
    for item in list_data:
        if item.Title == dataset:
            return item
    return None


if __name__ == '__main__':

    gxc = gx.GXpy()

    server = gxdap.DAPServer()
    server_catalog = gxdap.Catalog(server)
    server_dataset = gxdap.Dataset(server)
    server_service = gxdap.Service(server)

    list_of_datasets = server_catalog.search()
    for ds in list_of_datasets:
        print('{}::{}'.format(ds.Hierarchy, ds.Title))

    name = 'SRTM1 Kazakhstan'
    dataset = find_dataset(list_of_datasets, name)
    if dataset is None:
        raise Exception('{} not found'.format(name))

    # here is how to get a byte-stream of the image
    image_array = server_dataset.draw(dataset=dataset)
    image = Image.open(BytesIO(image_array))
    image.show()

    extents = server_dataset.extents(dataset=dataset)
    print(extents)

    properties = server_dataset.grid_properties(dataset=dataset)
    print(properties)

    metadata = gxu.dict_from_xml(server_dataset.metadata(dataset=dataset).decode('utf-8'))
    # print(metadata)

    res = server_dataset.extract_resolution(dataset=dataset)
    print(res)

    parameter = gxdapmodels.DefaultExtractionResolutionParameters(extents=dataset.Extents, type=dataset.Type)
    resolution = server_service.default_resolution(parameter)
    print(resolution)

    extract_parameters = gxdapmodels.GridExtract(extents=dataset.Extents, resolution=res.Default,
                                                 filename='Globe.grd(GRD)')
    key = server_dataset.extract_grid(id=dataset.Id, parameters=extract_parameters)
    print(key)

    progress = server_dataset.extract_progress(key=key)
    while (
            progress.Stage != gxdapmodels.ExtractProgressStatus.Complete and progress.Stage != gxdapmodels.ExtractProgressStatus.Cancelled and progress.Stage != gxdapmodels.ExtractProgressStatus.Failed):
        time.sleep(5)
        progress = server_dataset.extract_progress(key=key)

    information = server_dataset.extract_download_information(key=key)
    print(information)

    zip_file = server_dataset.extract_download(key=key, download_information=information, progress=print)

    print(zip_file)
