import pytest

async def test_get_partition_map(service_client):
    response = await service_client.get('/admin/partition_map')
    assert response.status == 200
    assert 'application/json' in response.headers['Content-Type']

    data = response.json()
    
    assert 'partition_map' in data
    assert 'partitions' in data['partition_map']
    assert 'epoch' in data['partition_map']

    partitions = data['partition_map']['partitions']
    assert len(partitions) == 100

    for partition in partitions:
        assert 'id' in partition
        assert isinstance(partition['id'], int)
        assert 'hub' in partition
        assert isinstance(partition['hub'], str)

    assert data['partition_map']['epoch'] == 0