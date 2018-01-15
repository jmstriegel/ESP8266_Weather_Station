import webapp2
import time
from datetime import datetime
import re
import json
from google.appengine.api import memcache

CACHETIME = 120

class UpdateHandler(webapp2.RequestHandler):
  def post(self):
    self.response.headers['Content-Type'] = 'text/json'

    data = {
      't': int(time.time()),
      '10_0': validPMData(self.request.get('pm10_0')),
      '2_5': validPMData(self.request.get('pm2_5')),
      '1_0': validPMData(self.request.get('pm1_0')),
      'rssi': validRSSI(self.request.get('rssi')),
      'ip': self.request.remote_addr,
    }
    device_id = validID(self.request.get('id'))

    key = 'device:%s' % device_id
    rawdata = memcache.get(key)
    devicedata = {
      'id': device_id,
      'latest': {},
      'history': []
    }
    if (rawdata is not None):
      devicedata = json.loads(rawdata)
    timebin = getTimeBinMinute(data['t'])
    devicedata['history'] = flushHistory(devicedata['history'], timebin)
    devicedata['history'].append(data)
    devicedata['latest'] = data

    memcache.set(key, json.dumps(devicedata), CACHETIME)

    key = 'activedevices'
    rawdata = memcache.get(key)
    devices = {}
    if (rawdata is not None):
      devices = json.loads(rawdata)
    devices[device_id] = data['t']
    devices = removeExpiredDevices(devices)
    memcache.set(key, json.dumps(devices), CACHETIME)

    result = {
      't': data['t']
    }
    self.response.out.write(json.dumps(result))


class ListHandler(webapp2.RequestHandler):
  def get(self):
    self.response.headers['Content-Type'] = 'text/json'
    devices = {}
    key = 'activedevices'
    rawdata = memcache.get(key)
    if (rawdata is not None):
      devices = json.loads(rawdata)
    devices = removeExpiredDevices(devices)
    device_arr = []
    for device_id, t in devices.iteritems():
      device_arr.append(device_id)
    response = {
      'devices': device_arr
    }
    self.response.out.write(json.dumps(response))


class StatusHandler(webapp2.RequestHandler):
  def get(self):
    self.response.headers['Content-Type'] = 'text/json'
    device_id = validID(self.request.get('id'))
    
    key = 'device:%s' % device_id
    rawdata = memcache.get(key)
    devicedata = {
      'id': device_id,
      'latest': {},
      'history': []
    }
    now = int(time.time())
    timediff = -1
    if (rawdata is not None):
      devicedata = json.loads(rawdata)
      timediff = now - devicedata['latest']['t']

    result = {
      'id': device_id,
      'latest': devicedata['latest'],
      'recency': timediff
    }
    self.response.out.write(json.dumps(result))
    
def getTimeBinMinute(timestamp):
  d = datetime.fromtimestamp(timestamp)
  return d.strftime('%Y%m%d%H%M')


def flushHistory(history, currenttimebin):
  bins = {}
  pruned = []
  for data in history:
    timebin = getTimeBinMinute(data['t'])
    if timebin == currenttimebin:
      pruned.append(data)
    else:
      if timebin not in bins:
        bins[timebin] = []
      bins[timebin].append(data)

  # TODO:  arrays in bins should be averaged and stored

  return pruned

def removeExpiredDevices(devices):
  newdata = {}
  cutoff = int(time.time()) - CACHETIME
  for device_id, t in devices.iteritems():
    if(t > cutoff):
      newdata[device_id] = t
  return newdata

def validPMData(data):
  if (data is None or data == ""):
    return -1
  else:
    return int(data)    

def validRSSI(data):
  if (data is None or data == ""):
    return 0
  else:
    return int(data)    

def validID(data):
  if (data is None or data == ""):
    return "default"
  else:
    return str(data)


app = webapp2.WSGIApplication(
  [
    ('/api/station/update', UpdateHandler),
    ('/api/station/list', ListHandler),
    ('/api/station/status', StatusHandler)
  ],
  debug=True
)
