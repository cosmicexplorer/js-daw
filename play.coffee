fs = require 'fs'
Speaker = require 'speaker'
spawn = require('child_process').spawn

speaker = new Speaker
  channels: 2
  bitDepth: 16
  sampleRate: 44100

pcm = spawn './createStream'

pcm.stdout.pipe speaker

pcm.stderr.on 'data', (msg) ->
  console.log msg.toString()

pcm.on 'close', ->
  console.log "peace"

setInterval((->
  # TODO: turn into json
  pcm.stdin.write "freq\n"
  pcm.kill('SIGUSR1'))
  ,2000)

setInterval((->
  pcm.stdin.write "quit\n"
  pcm.kill('SIGUSR1'))
  ,4000)
