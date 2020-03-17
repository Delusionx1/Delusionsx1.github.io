# https://requests.kennethreitz.org/en/master/
import requests
import uuid
from rdflib import Graph, Literal, BNode, Namespace, RDF, RDFS, XSD, URIRef

url = "http://rest.uwe.eslcloudsolutions.com/api/123/emstudy_uwe/register"
sosa = Namespace("http://www.w3.org/ns/sosa/")
eg = Namespace("http://www.example.com/")

g = Graph()
g.namespace_manager.bind('sosa', sosa)

# change this to your sensor node(s)
nodes = ["00:11:7d:00:00:40:23:59"]

post_body = {
  "emapID": "E148921E9CC54C2A8C6E0842539ACB3D",
  "dateTimeStart":"2020-03-17T00:00:00Z",
  "dateTimeEnd":"2020-03-17T23:59:00Z"
}

def isFloat(value):
  try:
    float(value)
    return True
  except ValueError:
    return False

def isInt(value):
  try:
    int(value)
    return True
  except ValueError:
    return False

def typedLiteral(v):
    if isInt(v):
        return Literal(v,datatype=XSD.integer)
    elif isFloat(v):
        return Literal(v,datatype=XSD.float)
    else:
        return Literal(v,datatype=XSD.string)

for nodeID in nodes:
  post_body["nodeID"] = nodeID
  response = requests.post(url, json=post_body)
  data = response.json();
  records = data["results"]["records"]
  if records:
    for r in records:
      dateTime = r.get('dateTime')
      message = r.get('message').split(',')[-1]
      nameValue = message.split('=')
      #print(dateTime,nameValue)

      # Name the subject with a universally unique identifier (large random number)
      subject = eg[str(uuid.uuid4())]
      g.add((subject, RDF.type, sosa.Observation))
      g.add((subject, sosa.hasSimpleResult, typedLiteral(nameValue[1])))
      blank = BNode()
      g.add((subject, sosa.madeBySensor, blank))
      g.add((blank, RDFS.label, Literal(nameValue[0])))
      
      dt = Literal(dateTime, datatype=XSD.dateTime)
      g.add((subject, sosa.resultTime, dt))

ttl = str(g.serialize(format='turtle'),'utf-8')
print(ttl)

with open("data.ttl", "w") as out:
    print(ttl, file=out)