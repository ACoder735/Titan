mod lexer;
mod parser;

use lexer::Lexer;

fn main() {
    let code = String::from("x := 5 + 3");
    let mut lexer = Lexer::new(code);
    
    loop {
        let token = lexer.next_token();
        println!("{:?}", token);
        if let lexer::Token::EOF = token {
            break;
        }
    }
}