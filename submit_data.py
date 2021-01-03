import requests

auth_token="<auth token>"
sensor_id="<sensor_id>

header = {"X-AUTH-TOKEN": auth_token, "Content-Type": "application/json"}

data = {
  "value": 42
}

url = 'https://datahub.010102.de/uv1/api/sensor/' + sensor_id + '/measurement'
response = requests.post(url, json=data, headers=header)
print(response)
