const express= require('express')
const bodyParser= require('body-parser')

const {insertTrk,findByDate}=require('./database')

const app=express()
const port = 3000
app.use(bodyParser.json())
app.post('/api',(req,res)=>{
    const obj=req.body
    insertTrk(obj)
    //console.log(obj)
    res.status(200).send('ok')

})


app.get('/api',async (req,res)=>{
    const {date} = req.query
    const temp= await findByDate(date)
    console.log(temp)
    res.status(200).json(temp) 
})

app.listen(port,(req,res)=>{
    console.log(`webserver is listening on port ${port}`)
})