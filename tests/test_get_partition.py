import pytest

async def test_get_partition(service_client):
    response = await service_client.get('/admin/partition?channel=test')
    assert response.status == 200
    assert 'application/json' in response.headers['Content-Type']
    assert response.text == '{"partition":6271892985061247544}'
