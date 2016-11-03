class A {
    public void blub(){
        /* 
         * Should both be parsed as LocalVariableDeclarationStatements 
         * as opposed to regular Statements and then failing.
         */
        int[] arr;
        String s = s;
    }
}
