# Start via `make test-debug` or `make test-release`


async def test_basic(service_client):
    response = await service_client.post('/hello', params={'name': 'Tester'})
    assert response.status == 200
    assert 'application/json' in response.headers['Content-Type']
    assert response.text == '{"partitions":[],"epoch":0}'