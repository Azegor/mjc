class Turing {
    public int[] tape;
    public int[][][] matrix;
    public int currentField;
    public int currentState;
    public int goalState;

    public static void main(String[] args) {
        Turing t = new Turing();

        t.run();
    }

    public void run() {
        TuringProgram mod2 = new TuringProgram();
        this.tape = mod2.getInitialTape();
        this.matrix = mod2.getMatrix();
        this.goalState = mod2.getGoalState();

        this.currentField = 1;
        this.currentState = 0;

        while (!nextStep()) {}

        int tapeLength = mod2.getTapeLength();
        int i = 0;
        while (i < tapeLength) {
            System.out.println(tape[i]);

            i = i + 1;
        }
    }

    /*
        Return "true" when program exited during simulation of the current step
     */
    public boolean nextStep() {
        int[] cell = this.matrix[this.currentState][this.tape[this.currentField]];
        int direction = cell[0];
        int state = cell[1];
        int newVal = cell[2];

        this.tape[this.currentField] = newVal;

        this.currentField = this.currentField + (direction - 1);

        this.currentState = state;

        if (currentState == this.goalState) {
            return true;
        }

        return false;
    }
}

/*
    This program detects whether a given number (as binary representation with MSB at lowest index) is odd or even. Has to be at least
    as long as 2 digits.
    The result returned is actually "mod 2".
 */
class TuringProgram {
    /*
        Program with len(voc) == 3 and len(states) == 3 and 3 directions

        int[<state>][<char>][<array-tripel containing direction, state, newVal>]

        Vocabulary:
        - 2: empty cell
        - 0: binary 0
        - 1: binary 1
        Directions:
        - 0: left
        - 1: stay
        - 2: right
     */
    public int[][][] getMatrix() {
        int[][][] matrix = new int[5][][];

        /*
            0: start state
            1: going rightwards, last digit read was 0
            2: going rightwards, last digit read was 1
            3: reached end, go leftwards in order to right a 0 as result
            4: reached end, go leftwards in order to right a 1 as result
         */
        int i = 0;
        while (i < 5) {
            matrix[i] = new int[3][];
            int j = 0;
            while (j < 3) {
                matrix[i][j] = new int[3];
                j = j + 1;
            }
            i = i + 1;
        }

        matrix[0][0][0] = 2; /* go right when we are in state 0 and read a binary 0 */
        matrix[0][0][1] = 1;
        matrix[0][0][2] = 0; /* we write a zero to the first position of the band to find our way back at the end */
        matrix[0][1][0] = 2;
        matrix[0][1][1] = 2;
        matrix[0][1][2] = 0;
        matrix[0][2][0] = 0;
        matrix[0][2][1] = getGoalState();
        matrix[0][2][2] = 0; /* no value mod 2 is 0 - by (our) definition */

        matrix[1][0][0] = 2;
        matrix[1][0][1] = 1;
        matrix[1][0][2] = 2;
        matrix[1][1][0] = 2;
        matrix[1][1][1] = 2;
        matrix[1][1][2] = 2;
        matrix[1][2][0] = 0;
        matrix[1][2][1] = 3;
        matrix[1][2][2] = 2;

        matrix[2][0][0] = 2;
        matrix[2][0][1] = 1;
        matrix[2][0][2] = 2;
        matrix[2][1][0] = 2;
        matrix[2][1][1] = 2;
        matrix[2][1][2] = 2;
        matrix[2][2][0] = 0;
        matrix[2][2][1] = 4;
        matrix[2][2][2] = 2;

        matrix[3][0][0] = 1; /* 0 indicates start position */
        matrix[3][0][1] = 5;
        matrix[3][0][2] = 0;
        matrix[3][1][0] = 0; /* passing a 1 on the way back cannot happen */
        matrix[3][1][1] = 3;
        matrix[3][1][2] = 2;
        matrix[3][2][0] = 0;
        matrix[3][2][1] = 3;
        matrix[3][2][2] = 2;

        matrix[4][0][0] = 1;
        matrix[4][0][1] = 5;
        matrix[4][0][2] = 1;
        matrix[4][1][0] = 0; /* passing a 1 on the way back cannot happen */
        matrix[4][1][1] = 2;
        matrix[4][1][2] = 2;
        matrix[4][2][0] = 0;
        matrix[4][2][1] = 4;
        matrix[4][2][2] = 2;

        return matrix;
    }

    public int getGoalState() {
        return 5;
    }

    public int[] getInitialTape() {
        int[] tape = new int[5];

        /*
            We need some empty space on both sides, this is just a workaround because we are lacking an infinite tape
         */
        tape[0] = 2;
        tape[1] = 1;
        tape[2] = 0;
        tape[3] = 1;
        tape[4] = 2;

        return tape;
    }

    public int getTapeLength() {
        return 5;
    }
}
/* RUN: %bindir/run --check %s
 */