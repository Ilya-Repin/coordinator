# Start via `make test-debug` or `make test-release`


async def test_basic(service_client):
    await asyncio.sleep(35)

    response = await service_client.post('/hello', params={'name': 'Tester'})
    assert response.status == 200
    assert 'application/json' in response.headers['Content-Type']
    assert response.text == '{"description":"Semaphore data"}'