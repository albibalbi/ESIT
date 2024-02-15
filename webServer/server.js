const express= require('express')
const bodyParser= require('body-parser')
const cors = require('cors')

const {insertTrk,findByDate,findAll}=require('./database')

const app=express()
const port = 3000  
//middleware for cors rules
app.use(cors({
    origin: "*", //allow access form any address --no restrictions
}))
//middleware for parsing request body to json
app.use(bodyParser.json())

//endpoint for sending data from ESP32 to database
app.post('/api',(req,res)=>{
    const obj=req.body
    insertTrk(obj)
    //console.log(obj)
    res.status(200).send('ok')

})

//endpoint used to get data from database to frontend
app.get('/api',async (req,res)=>{
    const date = req.query.date
    console.log(date)
    let temp
    if(date!=null){ //if is required a specific date 
        temp = await findByDate(date) // return a specific document on a specific date
    }
    else{
        temp= await findAll() // return all documents
    }
    res.set({'Content-type': 'application/json'})
    console.log(temp)
    res.status(200).json(temp) // set the status of the response in order to comunicate that the request well ended
})
//endpoint root used for testing
app.get('/', (req,res)=>{
    res.status(200).send(`<h1>CIAO</h1>`)
})
//start listening on port 3000 and set the ip mask so that every ip address is allowed to send requests
app.listen(port,'0.0.0.0',(req,res)=>{
    console.log(`webserver is listening on port ${port}`) 
})