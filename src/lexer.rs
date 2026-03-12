#![allow(dead_code)]

pub struct Lexer {
    source: String,
    pos: usize,
    line: usize,
}

#[derive(Debug)]
pub enum Token {
    Number(f64), 
    String(String), 
    Identifier(String),
    Func, If, Else, While, For, To, Match, Return, Enum,
    NumType, StrType, BoolType, True, False,
    ColonAssign, Plus, Minus, Star, Slash, Percent,
    Assign, Equal, NotEqual, Less, Greater, LessEqual, GreaterEqual,
    And, Or, Not, Arrow, PlusAssign, MinusAssign,
    LParen, RParen, LBrace, RBrace, LBracket, RBracket,
    Comma, Semicolon, Dot, EOF,
}

impl Lexer {
    pub fn new(source: String) -> Lexer {
        Lexer { source, pos: 0, line: 1 }
    }

    fn current(&self) -> char {
        if self.pos < self.source.len() {
            self.source.chars().nth(self.pos).unwrap()
        } else {
            '\0'
        }
    }

    fn peek(&self) -> char {
        if self.pos + 1 < self.source.len() {
            self.source.chars().nth(self.pos + 1).unwrap()
        } else {
            '\0'
        }
    }

    fn advance(&mut self) {
        self.pos += 1;
    }

    fn skip_whitespace(&mut self) {
        while self.current() == ' ' || self.current() == '\t' || self.current() == '\r' {
            self.advance();
        }
    }

    fn skip_comment(&mut self) {
        if self.current() == '/' && self.peek() == '/' {
            while self.current() != '\n' && self.current() != '\0' {
                self.advance();
            }
        }
    }

    pub fn next_token(&mut self) -> Token {
        loop {
            self.skip_whitespace();
            self.skip_comment();
            
            if self.current() == '\0' {
                return Token::EOF;
            }
            
            if self.current() == '\n' {
                self.line += 1;
                self.advance();
                continue;
            }
            
            break;
        }

        let c = self.current();
        self.advance();

        match c {
            '+' => {
                if self.current() == '=' { self.advance(); return Token::PlusAssign; }
                return Token::Plus;
            }
            '-' => {
                if self.current() == '>' { self.advance(); return Token::Arrow; }
                if self.current() == '=' { self.advance(); return Token::MinusAssign; }
                return Token::Minus;
            }
            '*' => return Token::Star,
            '/' => return Token::Slash,
            '%' => return Token::Percent,
            '(' => return Token::LParen,
            ')' => return Token::RParen,
            '{' => return Token::LBrace,
            '}' => return Token::RBrace,
            '[' => return Token::LBracket,
            ']' => return Token::RBracket,
            ',' => return Token::Comma,
            ';' => return Token::Semicolon,
            '.' => return Token::Dot,
            ':' => {
                if self.current() == '=' { self.advance(); return Token::ColonAssign; }
                return Token::EOF;
            }
            '=' => {
                if self.current() == '=' { self.advance(); return Token::Equal; }
                return Token::Assign;
            }
            '!' => {
                if self.current() == '=' { self.advance(); return Token::NotEqual; }
                return Token::Not;
            }
            '<' => {
                if self.current() == '=' { self.advance(); return Token::LessEqual; }
                return Token::Less;
            }
            '>' => {
                if self.current() == '=' { self.advance(); return Token::GreaterEqual; }
                return Token::Greater;
            }
            '&' => {
                if self.current() == '&' { self.advance(); return Token::And; }
                return Token::EOF;
            }
            '|' => {
                if self.current() == '|' { self.advance(); return Token::Or; }
                return Token::EOF;
            }
            '"' => {
                let mut s = String::new();
                while self.current() != '"' && self.current() != '\0' {
                    s.push(self.current());
                    self.advance();
                }
                self.advance();
                return Token::String(s);
            }
            '0'..='9' => {
                let mut num = String::new();
                num.push(c);
                while self.current() >= '0' && self.current() <= '9' {
                    num.push(self.current());
                    self.advance();
                }
                if self.current() == '.' {
                    num.push('.');
                    self.advance();
                    while self.current() >= '0' && self.current() <= '9' {
                        num.push(self.current());
                        self.advance();
                    }
                }
                return Token::Number(num.parse().unwrap());
            }
            'a'..='z' | 'A'..='Z' | '_' => {
                let mut name = String::new();
                name.push(c);
                while self.current() >= 'a' && self.current() <= 'z'
                    || self.current() >= 'A' && self.current() <= 'Z'
                    || self.current() >= '0' && self.current() <= '9'
                    || self.current() == '_' {
                    name.push(self.current());
                    self.advance();
                }
                return match name.as_str() {
                    "func" => Token::Func,
                    "if" => Token::If,
                    "else" => Token::Else,
                    "while" => Token::While,
                    "for" => Token::For,
                    "to" => Token::To,
                    "match" => Token::Match,
                    "return" => Token::Return,
                    "enum" => Token::Enum,
                    "Num" => Token::NumType,
                    "Str" => Token::StrType,
                    "Bool" => Token::BoolType,
                    "true" => Token::True,
                    "false" => Token::False,
                    _ => Token::Identifier(name),
                };
            }
            _ => return Token::EOF,
        }
    }
}