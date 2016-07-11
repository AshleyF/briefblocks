printfn "Brief Blocks 1.0"

let forward = function n :: stack -> printfn "Forward %i" n; stack | _ -> failwith "Stack underflow"
let left    = function n :: stack -> printfn "Left %i"    n; stack | _ -> failwith "Stack underflow"
let right   = function n :: stack -> printfn "Right %i"   n; stack | _ -> failwith "Stack underflow"

let num n stack = n :: stack

let blockToFunction = function
  | 0x01uy -> forward
  | 0x02uy -> left
  | 0x03uy -> right
  | 0x04uy -> num 0
  | 0x05uy -> num 1
  | 0x06uy -> num 2
  | 0x07uy -> num 3
  | 0x08uy -> num 4
  | 0x09uy -> num 5
  | 0x0auy -> num 6
  | 0x0buy -> num 7
  | 0x0cuy -> num 8
  | 0x0duy -> num 9
  | b -> sprintf "Unknown bytecode: %i" b |> failwith

let interpret code =
  List.map blockToFunction code
  |> List.fold (fun s f -> f s) []

let exec code =
  code |> interpret |> printfn "Stack: %A"

let F  = 0x01uy
let L  = 0x02uy
let R  = 0x03uy
let N0 = 0x04uy
let N1 = 0x05uy
let N2 = 0x06uy
let N3 = 0x07uy
let N4 = 0x08uy
let N5 = 0x09uy
let N6 = 0x0auy
let N7 = 0x0buy
let N8 = 0x0cuy
let N9 = 0x0duy

[N5; F] |> exec
