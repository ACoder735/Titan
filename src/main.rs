mod lexer;
mod parser;

use std::env;
use std::fs;
use lexer::Lexer;
use parser::Parser;

fn main() {
    let args: Vec<String> = env::args().collect();
    
    if args.len() < 2 {
        println!("Usage: titan <file.tn>");
        return;
    }
    
    let filename = &args[1];
    let code = fs::read_to_string(filename).expect("Can't read file");
    
    let lexer = Lexer::new(code);
    let mut parser = Parser::new(lexer);
    
    let ast = parser.parse();
    println!("{:?}", ast);
}