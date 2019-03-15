

FTP = {
    ftp : "hello"
};


a = {}
a.__proto__ = FTP;

a.ftp = "world";

console.log(FTP.ftp);
console.log(a.ftp);