from flask import Flask, render_template, request
import json
import subprocess
import sys
import os


class Judge:
    # exe path
    path = ""
    isStart = False

    def setmode(self, player1, player2):
        self.path = "python judge.py"
        if not player1:
            self.path += " ./mango "
        else:
            self.path += " human "
        if not player2:
            self.path += " ./mango "
        else:
            self.path += " human "

    def send(self, message):
        value = str(message) + '\n'
        value = bytes(value, 'UTF-8')
        self.proc.stdin.write(value)
        self.proc.stdin.flush()

    def receive(self):
        return self.proc.stdout.readline().strip().decode()

    def init(self):
        self.isStart = True
        self.proc = subprocess.Popen(self.path,
                                     stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE)

    # return [x,y,"opt"]
    # opt=-1 default
    # opt=0:0 win / opt=1:1 win
    # opt=2 draw
    def action(self, a, b):
        if a == -1 and b == 0:
            self.send("NEXT")
        else:
            self.send(str(a) + ' ' + str(b))
        ret = self.receive().split()
        ret[0] = int(ret[0])
        ret[1] = int(ret[1])
        if len(ret) == 3:
            return ret
        ret.append("-1")
        return ret

    def kill(self):
        if self.isStart:
            self.proc.kill()


app = Flask(__name__)
server = Judge()


@app.route('/')
def index():
    server.kill()
    return render_template("index.html")


@app.route('/kill', methods=["POST"])
def kill():
    server.kill()
    return ""


@app.route('/init', methods=["POST"])
def init():
    print(request.form)
    player1 = request.form.get("player1") == "true"
    player2 = request.form.get("player2") == "true"
    print(player1, player2)
    server.setmode(player1, player2)
    server.init()
    return ""


@app.route('/action', methods=["POST"])
def action():
    print(request.form)
    x = int(request.form.get("xaxis"))
    y = int(request.form.get("yaxis"))
    ret = server.action(x, y)
    if ret[2] != '-1':
        server.kill()
    return '{"xaxis":' + str(ret[0]) + ',' + '"yaxis":' + str(
        ret[1]) + ',' + '"result":' + ret[2] + '}'

    # if __name__ == "__main__":


app.run()