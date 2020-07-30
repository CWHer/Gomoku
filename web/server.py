from flask import Flask, render_template, request
import json
import subprocess
import sys
import os


class Judge:
    # exe path
    path = ""
    isStart = False
    chessset = []

    def setmode(self, player1, player2):
        self.path = "python judge.py "
        player = {0: "human ", 1: "./mango ", 2: "./baseline "}
        self.path += player[player1] + player[player2]

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
                                     stdout=subprocess.PIPE,
                                     start_new_session=True)

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
        if len(ret) != 2:
            if ret[2] != 2:
                self.chessset = ret[3:]
            return ret[0:3]
        ret.append("-1")
        return ret

    def kill(self):
        if self.isStart:
            self.isStart = False
            # linux
            # os.killpg(proc.pid, signal.SIGTERM)
            os.popen('taskkill  /t /f /pid ' + str(self.proc.pid))


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


@app.route('/chessset', methods=["POST"])
def chessset():
    pos = list(map(int, server.chessset))
    server.kill()
    ret = '{'
    for i in range(len(pos)):
        ret += '"num' + str(i) + '":' + str(pos[i])
        if i != len(pos) - 1:
            ret += ','
    return ret + '}'


@app.route('/init', methods=["POST"])
def init():
    print(request.form)
    switch = {"human": 0, "AI1": 1, "AI2": 2}
    player1 = switch[request.form.get("player1")]
    player2 = switch[request.form.get("player2")]
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
    return '{"xaxis":' + str(ret[0]) + ',' + '"yaxis":' + str(
        ret[1]) + ',' + '"result":' + ret[2] + '}'

    # if __name__ == "__main__":


app.run()