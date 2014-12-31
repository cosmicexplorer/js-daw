fs = require 'fs'
Speaker = require 'speaker'
spawn = require('child_process').spawn

speaker = new Speaker
  channels: 2
  bitDepth: 16
  sampleRate: 44100

pcm = spawn './createStream'

pcm.stdout.pipe speaker

streamClosed = false

pcm.stderr.on 'data', (msg) ->
  console.log msg.toString()
  if msg.toString() == "begin pcm\n"
    setTimeout((->
      # TODO: turn into json
      pcm.stdin.write "sa\n"
      pcm.kill 'SIGUSR1')
      ,200)

    setTimeout((->
      pcm.stdin.write "e\n"
      pcm.kill 'SIGUSR1')
      ,300)

    # setTimeout((->
    #   # TODO: turn into json
    #   pcm.stdin.write "sa\n"
    #   pcm.kill 'SIGUSR1')
    #   ,1000)

    setTimeout((->
      pcm.stdin.write "q\n"
      pcm.kill 'SIGUSR1')
      ,2000)


pcm.stdin.on 'close', ->
  streamClosed = true

pcm.on 'close', ->
  console.log "peace"

# ALL MESSAGES ON STDIN MUST END WITH A NEWLINE
# synths are stored in a list which can be accessed by indices
# stdin message code:
#   add: msg[0] == 'a'
#     square: msg[1] == 'q'
#     sin: msg[1] == 's'
#     triangle: msg[1] == 't'
#     frequency: msg[2->(length-1)] == freq
#   remove: msg[0] == 'r'
#     index: msg[1] == index
#   edit: msg[0] == 'e'
#     same as add, but with index at msg[1], all else pushed right one
#   clear: msg[0] == 'c'
#   quit thread: msg[0] == 'q'
