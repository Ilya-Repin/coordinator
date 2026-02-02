import pytest

async def test_get_partition_map(service_client):
    response = await service_client.get('/admin/partition_map')
    assert response.status == 200
    assert 'application/json' in response.headers['Content-Type']
    assert response.text == '{"partition_map":{"partitions":[],"epoch":0}}'
