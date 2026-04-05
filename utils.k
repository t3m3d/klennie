// utils.k — string utilities for ktop

// nthline: get nth newline-delimited line (0-indexed)
func nthline(s, idx) {
    let target = toInt(idx)
    let cur    = 0
    let start  = 0
    while "1" == "1" {
        let rest  = substring(s, start, len(s))
        let nlPos = indexOf(rest, "\n")
        if nlPos < 0 {
            if cur == target { emit trim(rest) }
            emit ""
        }
        if cur == target { emit trim(substring(rest, 0, nlPos)) }
        start = start + nlPos + 1
        cur   = cur + 1
    }
    emit ""
}

// linecount: count newline-delimited lines
func linecount(s) {
    if s == "" { emit "0" }
    let t = trim(s)
    if t == "" { emit "0" }
    let n = 1
    let i = 0
    while i < len(t) {
        if substring(t, i, i+1) == "\n" { n += 1 }
        i += 1
    }
    emit n + ""
}

// nthField: get nth whitespace-delimited field (0-indexed)
func nthField(s, idx) {
    let target = toInt(idx)
    let fi = 0
    let i  = 0
    let slen = len(s)
    while i <= slen {
        while i < slen && substring(s, i, i+1) == " " { i += 1 }
        let start = i
        while i < slen && substring(s, i, i+1) != " " { i += 1 }
        if start >= i { break }
        if fi == target { emit substring(s, start, i) }
        fi += 1
    }
    emit ""
}

// lastIndexOf: find last occurrence of sub in s
func lastIndexOf(s, sub) {
    let slen   = len(s)
    let sublen = len(sub)
    let last   = "-1"
    let i      = 0
    while i <= slen - sublen {
        if substring(s, i, i + sublen) == sub { last = i + "" }
        i += 1
    }
    emit last
}

// firstpart: text before first delimiter
func firstpart(s, delim) {
    let pos = indexOf(s, delim)
    if pos < 0 { emit s }
    emit substring(s, 0, pos)
}

// afterpart: text after first delimiter
func afterpart(s, delim) {
    let pos = indexOf(s, delim)
    if pos < 0 { emit "" }
    emit substring(s, pos + len(delim), len(s))
}

// strRepeat: repeat string s n times
func strRepeat(s, n) {
    let r = ""
    let i = 0
    while i < toInt(n) { r = r + s  i += 1 }
    emit r
}

// twoDigit: zero-pad integer string to 2 digits
func twoDigit(n) {
    if toInt(n) < 10 { emit "0" + n }
    emit n + ""
}

// removeLine: remove line at idx from newline-delimited string
func removeLine(s, idx) {
    let n      = toInt(linecount(s))
    let result = ""
    let i      = "0"
    while toInt(i) < n {
        if i != idx + "" {
            let line = nthline(s, i)
            if result != "" { result = result + "\n" }
            result = result + line
        }
        i = toInt(i) + 1 + ""
    }
    emit result
}

// appendLine: append a line to a newline-delimited string
func appendLine(s, line) {
    if s == "" { emit line }
    emit s + "\n" + line
}
