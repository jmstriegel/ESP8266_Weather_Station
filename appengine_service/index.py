import json
import os
import pickle
import webapp2
from google.appengine.ext.webapp import template

class IndexHandler( webapp2.RequestHandler ):

  def get( self ):
    self.response.headers['Content-Type'] = 'text/html'
    tmpl = os.path.join(os.path.dirname(__file__), 'views/index.html')
    tmplvars = {
      'device_id': ''
    }
    device_id = self.request.get('id')
    if device_id is not None:
      tmplvars['device_id'] = device_id

    self.response.out.write(template.render(tmpl, tmplvars))

app = webapp2.WSGIApplication(
  [
    ('/', IndexHandler)
  ],
  debug=True
)
