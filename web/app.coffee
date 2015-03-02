path = require 'path'
express = require 'express'
app = express()
app.use require('morgan')('dev')
app.use express.static(path.join(__dirname, 'public'))
app.use require('errorhandler')() if app.get('env') == 'development'

port = 3000
require('http').createServer(app).listen port, ->
  console.log "Express server listening on port " + port
