#![allow(dead_code)]

use crate::lexer::{Lexer, Token};

#[derive(Debug)]
pub enum AST {
    Program(Vec<AST>),
    VarDecl { name: String, value: Box<AST> },
    Assign { name: String, value: Box<AST> },
    Number(f64), String(String), Identifier(String),
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
        match &self.current_token {
            Token::If => {
                self.advance();
                let condition = self.parse_expression();
                let body = self.parse_block();
                let mut else_body = None;

                if matches!(self.current_token, Token::Else) {
                    self.advance();

                    if matches!(self.current_token, Token::If) {
                        else_body = Some(vec![self.parse_statement()]);
                    } else {
                        else_body = Some(self.parse_block());
                    }
                }

                return AST::If {
                    condition: Box::new(condition),
                    body,
                    else_body,
                };
            }
            Token::While => {
                self.advance();
                let condition = self.parse_expression();
                let body = self.parse_block();
                return AST::While {
                    condition: Box::new(condition),
                    body,
                };
            }
            Token::For => {
                self.advance();

                let var = match &self.current_token {
                    Token::Identifier(name) => {
                        let n = name.clone();
                        self.advance();
                        n
                    }
                    _ => String::new(),
                };

                if matches!(self.current_token, Token::ColonAssign) {
                    self.advance();
                }

                let start = self.parse_expression();

                if matches!(self.current_token, Token::To) {
                    self.advance();
                }

                let end = self.parse_expression();
                let body = self.parse_block();

                return AST::For {
                    var,
                    start: Box::new(start),
                    end: Box::new(end),
                    body,
                };
            }
            Token::Identifier(name) => {
                let var_name = name.clone();
                self.advance();

                match &self.current_token {
                    Token::ColonAssign => {
                        self.advance();
                        let value = self.parse_expression();
                        return AST::VarDecl {
                            name: var_name,
                            value: Box::new(value),
                        };
                    }
                    Token::Assign => {
                        self.advance();
                        let value = self.parse_expression();
                        return AST::Assign {
                            name: var_name,
                            value: Box::new(value),
                        };
                    }
                    Token::LParen => {
                        self.advance();
                        let args = self.parse_args();
                        return AST::FunctionCall { name: var_name, args };
                    }
                    _ => return AST::Identifier(var_name),
                }
            }
            _ => {
                self.advance();
                return AST::Number(0.0);
            }
        }
    }

    fn parse_block(&mut self) -> Vec<AST> {
        let mut statements = Vec::new();

        if matches!(self.current_token, Token::LBrace) {
            self.advance();
        }

        while !matches!(self.current_token, Token::RBrace | Token::EOF) {
            statements.push(self.parse_statement());
        }

        if matches!(self.current_token, Token::RBrace) {
            self.advance();
        }

        statements
    }

    fn parse_expression(&mut self) -> AST {
        self.parse_comparison()
    }

    fn parse_comparison(&mut self) -> AST {
        let mut left = self.parse_term();

        while matches!(
            self.current_token,
            Token::Greater
                | Token::Less
                | Token::Equal
                | Token::NotEqual
                | Token::GreaterEqual
                | Token::LessEqual
        ) {
            let op = match &self.current_token {
                Token::Greater => ">".to_string(),
                Token::Less => "<".to_string(),
                Token::Equal => "==".to_string(),
                Token::NotEqual => "!=".to_string(),
                Token::GreaterEqual => ">=".to_string(),
                Token::LessEqual => "<=".to_string(),
                _ => unreachable!(),
            };
            self.advance();
            let right = self.parse_term();
            left = AST::BinaryOp {
                left: Box::new(left),
                op,
                right: Box::new(right),
            };
        }

        left
    }

    fn parse_term(&mut self) -> AST {
        let mut left = self.parse_factor();

        while matches!(self.current_token, Token::Plus | Token::Minus) {
            let op = match &self.current_token {
                Token::Plus => "+".to_string(),
                Token::Minus => "-".to_string(),
                _ => unreachable!(),
            };
            self.advance();
            let right = self.parse_factor();
            left = AST::BinaryOp {
                left: Box::new(left),
                op,
                right: Box::new(right),
            };
        }

        left
    }

    fn parse_factor(&mut self) -> AST {
        let mut left = self.parse_primary();

        while matches!(
            self.current_token,
            Token::Star | Token::Slash | Token::Percent
        ) {
            let op = match &self.current_token {
                Token::Star => "*".to_string(),
                Token::Slash => "/".to_string(),
                Token::Percent => "%".to_string(),
                _ => unreachable!(),
            };
            self.advance();
            let right = self.parse_primary();
            left = AST::BinaryOp {
                left: Box::new(left),
                op,
                right: Box::new(right),
            };
        }

        left
    }

    fn parse_primary(&mut self) -> AST {
        match &self.current_token {
            Token::Number(n) => {
                let val = *n;
                self.advance();
                AST::Number(val)
            }
            Token::String(s) => {
                let val = s.clone();
                self.advance();
                AST::String(val)
            }
            Token::Identifier(name) => {
                let n = name.clone();
                self.advance();
                AST::Identifier(n)
            }
            Token::LParen => {
                self.advance();
                let expr = self.parse_expression();
                if let Token::RParen = self.current_token {
                    self.advance();
                }
                expr
            }
            _ => AST::Number(0.0),
        }
    }

    fn parse_args(&mut self) -> Vec<AST> {
        let mut args = Vec::new();

        if let Token::RParen = self.current_token {
            self.advance();
            return args;
        }

        args.push(self.parse_expression());

        while let Token::Comma = self.current_token {
            self.advance();
            args.push(self.parse_expression());
        }

        if let Token::RParen = self.current_token {
            self.advance();
        }

        args
    }
}