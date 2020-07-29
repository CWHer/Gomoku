var gomoku = {
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
        this.chessBoardHtml = $(".chessboard").html();
    },

    run: function () {
        this.reset();
        this.main();
    },

    reset: function () {
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
        $("#swap").attr("disabled", "true");
        if (col == "black" && !gomoku.isSwap) {
            this.turn++;
            if (gomoku.turn == 2)
                $("#swap").removeAttr("disabled");
        }
        gomoku.wait = 1; //lock
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
                data = $.parseJSON(data);
                var i = data["xaxis"],
                    j = data["yaxis"];
                //flip:-1
                if (col == "white" && i != -1)
                    gomoku.whiteLastChess = [i, j];
                if (col == "black")
                    gomoku.blackLastChess = [i, j];
                if (i == -1) {
                    gomoku.flip();
                    if (!gomoku.isPlayer[1]) gomoku.AIaction("black");
                    return;
                }
                gomoku.isPlayerTurn[0] ^= 1;
                gomoku.isPlayerTurn[1] ^= 1;
                gomoku.putChess(i, j, col);
                if (data["result"] != "-1") {
                    gomoku.isGameOver = 1;
                    // gomoku.checkWin();
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
                "player1": this.isPlayer[0],
                "player2": this.isPlayer[1]
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
                            gomoku.isGameOver = 1;
                            // gomoku.checkWin();
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
    checkWin: function () {

    },
    //flip board
    flip: function () {
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