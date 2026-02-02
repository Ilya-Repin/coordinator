# Start via `make test-debug` or `make test-release`


async def test_get_context(service_client):
    response = await service_client.get('/admin/context')
    assert response.status == 200
    assert 'application/json' in response.headers['Content-Type']
    assert response.text == '{"context":{"partitions_context":[]}}'
