const express= require('express')
const bodyParser= require('body-parser')
const cors = require('cors')

const {insertTrk,findByDate,findAll}=require('./database')

const app=express()
const port = 3000
app.use(cors({
    origin: "http://13.60.21.191:4000/",
}))
app.use(bodyParser.json())

app.post('/api',(req,res)=>{
    const obj=req.body
    insertTrk(obj)
    //console.log(obj)
    res.status(200).send('ok')

})


app.get('/api',async (req,res)=>{
    const date = req.query.date
    console.log(date)
    let temp
    if(date!=null){
        temp = await findByDate(date)
    }
    else{
        temp= await findAll()
    }
    res.set({'Content-type': 'application/json'})
    console.log(temp)
    res.status(200).json(temp) 
})
app.get('/', (req,res)=>{
    res.status(200).send(`<h1>CIAO</h1>`)
})

app.listen(port,'0.0.0.0',(req,res)=>{
    console.log(`webserver is listening on port ${port}`)
})