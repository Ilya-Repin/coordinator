import pytest

async def test_get_hub_reports(service_client):
    response = await service_client.get('/admin/hub_reports')
    assert response.status == 200
    assert 'application/json' in response.headers['Content-Type']
    assert response.text == '{"hub_reports":[]}'
