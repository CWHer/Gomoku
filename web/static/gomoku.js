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
    isPlayerTurn: false,
    isGameStart: false,
    isGameOver: false,

    init: function () {
        this.chessBoardHtml = $(".chessboard").html();
        this.reset();
    },

    reset: function () {
        $(".chessboard").html(this.chessBoardHtml);
        this.isPlayerTurn = this.FirstisPlayer = $("#first-player").text() == "human";
        this.SecondisPlayer = $("#second-player").text() == "human";
        this.isGameOver = false;
        //reset board
        for (var i = 0; i < 15; i++) {
            this.chessArr[i] = [];
            for (var j = 0; j < 15; j++) {
                this.chessArr[i][j] = this.NO_CHESS;
            }
        }
        $(".chessboard div").click(function () {
            if (!gomoku.FirstisPlayer || gomoku.isGameOver)
                return;
            if (!gomoku.isGameStart)
                gomoku.gameStart();
            var index = $(this).index(),
                i = index / 15 | 0,
                j = index % 15;
            gomoku.putChess(i, j, "black");
            $(this).removeClass("hover");
        })
        $(".chessboard div").hover(
            function () {
                if (!gomoku.isPlayerTurn || gomoku.isGameOver) {
                    return;
                }
                var index = $(this).index(),
                    i = index / 15 | 0,
                    j = index % 15;
                if (gomoku.chessArr[i][j] == gomoku.NO_CHESS)
                    $(this).addClass("hover");
            },
            function () {
                if (!gomoku.isPlayerTurn || gomoku.isGameOver) {
                    return;
                }
                var index = $(this).index(),
                    i = index / 15 | 0,
                    j = index % 15;
                if (gomoku.chessArr[i][j] == gomoku.NO_CHESS)
                    $(this).removeClass("hover");
            }
        );
    },

    gameStart: function () {

        $.post("/init", {
            black: this.FirstisPlayer,
            white: this.SecondisPlayer,
        });
        //AI first
        if (!this.FirstisPlayer) {
            $.post("/action", {
                color: "balck",
                side: "AI"
            });
        }
        this.isGameStart = true;
    },

    putChess: function (i, j, color) {
        this.chessArr[i][j] = color == "black" ? this.BLACK_CHESS : this.WHITE_CHESS;
        $("div.chessboard div." + color + "-cur").addClass(color).removeClass(color + "-cur");
        $("div.chessboard div:eq(" + (i * 15 + j) + ")").addClass(color + "-cur");
    }
}