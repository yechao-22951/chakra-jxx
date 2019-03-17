

FTP = {
    ftp : "hello"
};


a = {}
a.__proto__ = FTP;

a.ftp = "world";

let xx = Symbol(1);
let yy = Symbol(2);

console.log( xx < yy );

console.log(FTP.ftp);
console.log(a.ftp);