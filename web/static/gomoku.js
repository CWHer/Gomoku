var gomoku = {
    turncnt: 0,
    turn: 0,
    isSwap: 0,
    wait: 0,
    NO_CHESS: -1,
    BLACK_CHESS: 0,
    WHITE_CHESS: 1,
    chessArr: [],
    chessBoardHtml: "",
    blackLastChess: [],
    whiteLastChess: [],
    isPlayer: [true, false], //human:1 AI:0
    isPlayerTurn: [true, false], //current is which player turn
    isGameStart: false,
    isGameOver: false,

    init: function () {
        this.turncnt = 0
        this.chessBoardHtml = $(".chessboard").html();
    },

    run: function () {
        this.reset();
        this.main();
    },

    reset: function () {
        $(".alert-warning").addClass("hide");
        $(".alert-success").addClass("hide");
        this.turn = this.wait = this.isSwap = 0;
        $(".chessboard").html(this.chessBoardHtml);
        this.isPlayer[0] = $("#first-player").text() == "human";
        this.isPlayer[1] = $("#second-player").text() == "human";
        this.isGameOver = false;
        this.isPlayerTurn[0] = this.isPlayerTurn[1] = false;
        //reset board
        for (var i = 0; i < 15; i++) {
            this.chessArr[i] = [];
            for (var j = 0; j < 15; j++) {
                this.chessArr[i][j] = this.NO_CHESS;
            }
        }
    },
    //request AI action
    AIaction: function (col) {
        $("#AIbar").css("width", "0%");
        $("#swap").attr("disabled", "true");
        if (col == "black" && !gomoku.isSwap) {
            this.turn++;
            if (gomoku.turn == 2)
                $("#swap").removeAttr("disabled");
        }
        if (col == "white" && gomoku.isSwap)
            this.turn++;
        gomoku.wait = 1; //lock
        $("#AIbar").css("width", "30%");
        $.post("/action", {
                "xaxis": -1,
                "yaxis": 0
            },
            function (data, status) {
                if (status != "success") {
                    alert("AI action failed");
                    return;
                }
                gomoku.wait = 0; //delock
                $("#AIbar").css("width", "80%");
                data = $.parseJSON(data);
                var i = data["xaxis"],
                    j = data["yaxis"];
                //flip:-1
                if (col == "white" && i != -1)
                    gomoku.whiteLastChess = [i, j];
                if (col == "black")
                    gomoku.blackLastChess = [i, j];
                $("#AIbar").css("width", "100%");
                if (i == -1) {
                    gomoku.flip();
                    if (!gomoku.isPlayer[1]) gomoku.AIaction("black");
                    return;
                }
                gomoku.isPlayerTurn[0] ^= 1;
                gomoku.isPlayerTurn[1] ^= 1;
                gomoku.putChess(i, j, col);
                if (data["result"] != "-1") {
                    gomoku.turncnt++;
                    gomoku.isGameOver = 1;
                    if (data["result"] != 2) gomoku.markWin(col);
                    //append result
                    var txt = '<div class="alert alert-success alert-dismissible" disabled="true">';
                    txt += '<button type="button" class="close" data-dismiss="alert">&times;</button>';
                    txt += '<strong>';
                    if (data["result"] != 2)
                        $(".resultalert").append(txt + '玩家' + (data["result"] + 1) + '</strong>获胜!</div>');
                    else
                        $(".resultalert").append(txt + '平局</strong></div>');
                    //flush
                    if (gomoku.turncnt % 5 == 1)
                        $("#record").empty();
                    //append record
                    if (!gomoku.isPlayer[0] && !gomoku.isPlayer[1])
                        txt = '<div class="panel panel-success">';
                    else
                        txt = '<div class="panel panel-danger">';
                    txt += '<div class="panel-heading"><h4 class="panel-title"> <a data-toggle="collapse" data-parent="#record" href="#collapse';
                    txt += gomoku.turncnt + '">对局记录</a> </h4></div><div id="collapse' + gomoku.turncnt + '" class="panel-collapse collapse"><div class="panel-body"><ul class="breadcrumb">'
                    txt += '<li class="active"><strong>' + $("#first-player").text() + "</strong>对战<strong>" + $("#second-player").text() + '</strong></li>';
                    txt += '<li class="active">回合数:<strong>' + gomoku.turn + '</strong></li>';
                    if (data["result"] != 2)
                        txt += '<li class="active"><strong>玩家' + (data["result"] + 1) + '</strong>获胜!</li>';
                    else
                        txt += '<li class="active"><strong>平局</strong></li>';
                    txt += '</ul></div></div></div>';
                    $("#record").append(txt);
                }
                if (gomoku.isGameOver) return;
                if (col == "black")
                    if (!gomoku.isPlayer[1 ^ gomoku.isSwap])
                        gomoku.AIaction("white");
                if (col == "white")
                    if (!gomoku.isPlayer[0 ^ gomoku.isSwap])
                        gomoku.AIaction("black");
            });
    },

    main: function () {
        $.post("/init", {
                "player1": $("#first-player").text(),
                "player2": $("#second-player").text()
            },
            function (data, status) {
                if (status != "success") {
                    alert("AI action failed");
                    return;
                }
                gomoku.isGameStart = true;
                gomoku.isPlayerTurn[0] = true;
                //AI first
                if (!gomoku.isPlayer[0]) {
                    gomoku.AIaction("black");
                }
            });
        // similar to judge.py
        // player action
        $(".chessboard div").click(function () {
            if (gomoku.wait) return;
            if (gomoku.isGameOver || !gomoku.isGameStart)
                return;
            if (gomoku.isPlayerTurn[0] && !gomoku.isPlayer[0])
                return;
            if (gomoku.isPlayerTurn[1] && !gomoku.isPlayer[1])
                return;
            var index = $(this).index(),
                i = index / 15 | 0,
                j = index % 15;
            var col;
            if (gomoku.chessArr[i][j] === gomoku.NO_CHESS) {
                if ((gomoku.isPlayer[0] && gomoku.isPlayerTurn[0])) {
                    col = gomoku.isSwap ? "white" : "black";
                    gomoku.turn++;
                    gomoku.putChess(i, j, col);
                    gomoku.blackLastChess = [i, j];
                    if (gomoku.turn == 2)
                        $("#swap").removeAttr("disabled");
                } else if ((gomoku.isPlayer[1] && gomoku.isPlayerTurn[1])) {
                    col = gomoku.isSwap ? "black" : "white";
                    gomoku.putChess(i, j, col);
                    gomoku.whiteLastChess = [i, j];
                    $("#swap").attr("disabled", "true");
                }
                $(this).removeClass("hover");
                //player action -> server
                gomoku.wait = 1;
                $.post("/action", {
                        "xaxis": i,
                        "yaxis": j
                    },
                    function (data, status) {
                        if (status != "success") {
                            alert("player action failed");
                            return;
                        }
                        gomoku.wait = 0;
                        data = $.parseJSON(data);
                        gomoku.isPlayerTurn[0] ^= 1;
                        gomoku.isPlayerTurn[1] ^= 1;
                        if (data["result"] != "-1") {
                            gomoku.turncnt++;
                            gomoku.isGameOver = 1;
                            if (data["result"] != 2) gomoku.markWin(col);
                            //append result
                            var txt = '<div class="alert alert-success alert-dismissible" disabled="true">';
                            txt += '<button type="button" class="close" data-dismiss="alert">&times;</button>';
                            txt += '<strong>';
                            if (data["result"] != 2)
                                $(".resultalert").append(txt + '玩家' + (data["result"] + 1) + '</strong>获胜!</div>');
                            else
                                $(".resultalert").append(txt + '平局</strong></div>');
                            //flush
                            if (gomoku.turncnt % 5 == 1)
                                $("#record").empty();
                            //append record
                            txt = '<div class="panel panel-success"> <div class="panel-heading"><h4 class="panel-title"> <a data-toggle="collapse" data-parent="#record" href="#collapse';
                            txt += gomoku.turncnt + '">对局记录</a> </h4></div><div id="collapse' + gomoku.turncnt + '" class="panel-collapse collapse"><div class="panel-body"><ul class="breadcrumb">'
                            txt += '<li class="active"><strong>' + $("#first-player").text() + "</strong>对战<strong>" + $("#second-player").text() + '</strong></li>';
                            txt += '<li class="active">回合数:<strong>' + gomoku.turn + '</strong></li>';
                            if (data["result"] != 2)
                                txt += '<li class="active"><strong>玩家' + (data["result"] + 1) + '</strong>获胜!</li>';
                            else
                                txt += '<li class="active"><strong>平局</strong></li>';
                            txt += '</ul></div></div></div>';
                            $("#record").append(txt);
                        }
                        if (gomoku.isGameOver) return;
                        if (col == "black")
                            if (!gomoku.isPlayer[1 ^ gomoku.isSwap])
                                gomoku.AIaction("white");
                        if (col == "white")
                            if (!gomoku.isPlayer[0 ^ gomoku.isSwap])
                                gomoku.AIaction("black");
                    });
            }
        })
        // player hover
        $(".chessboard div").hover(
            function () {
                if (gomoku.wait) return;
                if (gomoku.isGameOver || !gomoku.isGameStart)
                    return;
                if (gomoku.isPlayerTurn[0] && !gomoku.isPlayer[0])
                    return;
                if (gomoku.isPlayerTurn[1] && !gomoku.isPlayer[1])
                    return;
                var index = $(this).index(),
                    i = index / 15 | 0,
                    j = index % 15;
                if (gomoku.chessArr[i][j] == gomoku.NO_CHESS)
                    $(this).addClass("hover");
            },
            function () {
                if (gomoku.wait) return;
                if (gomoku.isGameOver || !gomoku.isGameStart)
                    return;
                if (gomoku.isPlayerTurn[0] && !gomoku.isPlayer[0])
                    return;
                if (gomoku.isPlayerTurn[1] && !gomoku.isPlayer[1])
                    return;
                var index = $(this).index(),
                    i = index / 15 | 0,
                    j = index % 15;
                if (gomoku.chessArr[i][j] == gomoku.NO_CHESS)
                    $(this).removeClass("hover");
            }
        );
    },
    //put on chess
    putChess: function (i, j, color) {
        if (i == -1 && j == -1) {
            this.flip();
            return;
        }
        this.chessArr[i][j] = color == "black" ? this.BLACK_CHESS : this.WHITE_CHESS;
        $("div.chessboard div.black-cur").addClass("black").removeClass("black-cur");
        $("div.chessboard div.white-cur").addClass("white").removeClass("white-cur");
        $("div.chessboard div:eq(" + (i * 15 + j) + ")").addClass(color + "-cur");
    },
    //mark chess when 5 in a row
    markChess: function (i, j, color) {
        $("div.chessboard div:eq(" + (i * 15 + j) + ")").removeClass(color).removeClass(color + "-cur").addClass(color + "-last");
    },
    markWin: function (col) {
        $.post("/chessset",
            function (data, status) {
                if (status != "success") {
                    alert("markwin failed");
                    return;
                }
                data = $.parseJSON(data);
                for (var i = 0; i < 5; ++i)
                    gomoku.markChess(data["num" + (i * 2)], data["num" + (i * 2 + 1)], col);
            });
    },
    //flip board
    flip: function () {
        var txt = '<div class="alert alert-warning alert-dismissible" disabled="true">';
        txt += '<button type="button" class="close" data-dismiss="alert">&times;</button>';
        txt += '<strong>玩家2选择交换颜色! </strong></div>';
        $(".swapalert").append(txt);
        this.isSwap = 1;
        var t1 = this.isPlayer[0];
        this.isPlayer[0] = this.isPlayer[1];
        this.isPlayer[1] = t1;
        for (var i = 0; i < 15; i++) {
            for (var j = 0; j < 15; j++) {
                if (this.chessArr[i][j] == this.NO_CHESS) continue;
                this.chessArr[i][j] ^= 1;
            }
        }
        $("div.chessboard div.black").addClass("white").removeClass("black");
        $("div.chessboard div:eq(" + (
            this.blackLastChess[0] * 15 + this.blackLastChess[1]) + ")").addClass("white").removeClass("black-cur");
        $("div.chessboard div:eq(" + (
            this.whiteLastChess[0] * 15 + this.whiteLastChess[1]) + ")").addClass("black").removeClass("white");
        var t2 = this.blackLastChess;
        this.blackLastChess = this.whiteLastChess;
        this.whiteLastChess = t2;
    }
}