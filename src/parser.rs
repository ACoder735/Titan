use crate::lexer::{Lexer, Token};

#[derive(Debug)]
pub enum AST {
    Program(Vec<AST>),
    VarDecl { name: String, value: Box<AST> },
    Assign { name: String, value: Box<AST> },
    Number(f64),
    String(String),
    Identifier(String),
    BinaryOp { left: Box<AST>, op: String, right: Box<AST> },
    If { condition: Box<AST>, body: Vec<AST>, else_body: Option<Vec<AST>> },
    While { condition: Box<AST>, body: Vec<AST> },
    For { var: String, start: Box<AST>, end: Box<AST>, body: Vec<AST> },
    FunctionCall { name: String, args: Vec<AST> },
}

pub struct Parser {
    lexer: Lexer,
    current_token: Token,
}

impl Parser {
    pub fn new(mut lexer: Lexer) -> Parser {
        let token = lexer.next_token();
        Parser { lexer, current_token: token }
    }

    fn advance(&mut self) {
        self.current_token = self.lexer.next_token();
    }

    pub fn parse(&mut self) -> AST {
        let mut statements = Vec::new();
        
        loop {
            match &self.current_token {
                Token::EOF => break,
                _ => statements.push(self.parse_statement()),
            }
        }
        
        AST::Program(statements)
    }

    fn parse_statement(&mut self) -> AST {
        // TODO
        AST::Number(0.0)
    }
}