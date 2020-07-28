var gomoku = {
    NO_CHESS: -1,
    BLACK_CHESS: 0,
    WHITE_CHESS: 1,
    chessArr: [],
    chessBoardHtml: "",
    blackLastChess: [],
    whiteLastChess: [],
    FirstisPlayer: true,
    SecondisPlayer: true,
    isPlayer1: false,
    isPlayer2: false,
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
        $(".chessboard").html(this.chessBoardHtml);
        this.isPlayerTurn = this.FirstisPlayer = $("#first-player").text() == "human";
        this.SecondisPlayer = $("#second-player").text() == "human";
        this.isGameOver = false;
        this.isPlayer1 = this.isPlayer2 = false;
        //reset board
        for (var i = 0; i < 15; i++) {
            this.chessArr[i] = [];
            for (var j = 0; j < 15; j++) {
                this.chessArr[i][j] = this.NO_CHESS;
            }
        }
    },

    main: function () {
        // $.post("/init", {
        //     black: this.FirstisPlayer,
        //     white: this.SecondisPlayer,
        // });
        this.isGameStart = true;
        this.isPlayer1 = true;
        // similar to judge.py
        var turn = 0;
        $(".chessboard div").click(function () {
            if (gomoku.isGameOver)
                return;
            if (gomoku.isPlayer1 && !gomoku.FirstisPlayer)
                return;
            if (gomoku.isPlayer2 && !gomoku.SecondisPlayer)
                return;
            var index = $(this).index(),
                i = index / 15 | 0,
                j = index % 15;
            if (gomoku.chessArr[i][j] === gomoku.NO_CHESS) {
                if ((gomoku.FirstisPlayer && gomoku.isPlayer1)) {
                    turn++;
                    gomoku.putChess(i, j, "black");
                    gomoku.blackLastChess = [i, j];
                    if (turn == 2)
                        $("#swap").removeAttr("disabled");
                }
                if ((gomoku.SecondisPlayer && gomoku.isPlayer2)) {
                    gomoku.putChess(i, j, "white");
                    gomoku.whiteLastChess = [i, j];
                    $("#swap").attr("disabled", "true");
                }

                $(this).removeClass("hover");
            }
            gomoku.isPlayer1 ^= 1;
            gomoku.isPlayer2 ^= 1;
        })
        $(".chessboard div").hover(
            function () {
                if (gomoku.isGameOver)
                    return;
                if (gomoku.isPlayer1 && !gomoku.FirstisPlayer)
                    return;
                if (gomoku.isPlayer2 && !gomoku.SecondisPlayer)
                    return;
                var index = $(this).index(),
                    i = index / 15 | 0,
                    j = index % 15;
                if (gomoku.chessArr[i][j] == gomoku.NO_CHESS)
                    $(this).addClass("hover");
            },
            function () {
                if (gomoku.isGameOver)
                    return;
                if (gomoku.isPlayer1 && !gomoku.FirstisPlayer)
                    return;
                if (gomoku.isPlayer2 && !gomoku.SecondisPlayer)
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
        this.chessArr[i][j] = color == "black" ? this.BLACK_CHESS : this.WHITE_CHESS;
        $("div.chessboard div.black-cur").addClass("black").removeClass("black-cur");
        $("div.chessboard div.white-cur").addClass("white").removeClass("white-cur");
        $("div.chessboard div:eq(" + (i * 15 + j) + ")").addClass(color + "-cur");
    },
    //mark chess when 5 in a row
    markChess: function (i, j, color) {
        $("div.chessboard div:eq(" + (i * 15 + j) + ")").removeClass(color).removeClass(color + "-cur").addClass(color + "-last");
    },
    //flip board
    flip: function () {
        for (var i = 0; i < 15; i++) {
            for (var j = 0; j < 15; j++) {
                if (this.chessArr[i][j] == this.NO_CHESS) continue;
                this.chessArr[i][j] ^= 1;
            }
        }
        $("div.chessboard div.black").addClass("white").removeClass("black");
        $("div.chessboard div:eq(" + (
            gomoku.blackLastChess[0] * 15 + gomoku.blackLastChess[1]) + ")").addClass("white").removeClass("black-cur");
        $("div.chessboard div:eq(" + (
            gomoku.whiteLastChess[0] * 15 + gomoku.whiteLastChess[1]) + ")").addClass("black").removeClass("white");
    }
}