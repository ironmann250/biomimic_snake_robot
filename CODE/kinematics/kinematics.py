import math

def d(a,q):
    q=math.radians(q)
    _90=math.radians(90)
    return a-(a*math.cos(_90-q))

def f(a,q):
    q=math.radians(q)
    _90=math.radians(90)
    return a*math.sqrt(2*(1-math.cos(_90-q)))

def bt(a,q):
    _90=math.radians(90)
    q=math.radians(q)
    top=math.sin(_90-q)*math.sqrt((1+math.sin(90-q))*((1-math.sin(90-q))))
    bottom=(1+math.sin(90-q))*((1-math.sin(90-q)))
    return math.degrees(math.atan(top/bottom))

def to(a,q):
    _90=math.radians(90)
    q=math.radians(q)
    top=1-math.cos(_90-q)
    bottom=math.sin(90-q)
    return ((_90-q)/2)
    #return math.degrees(_90-(q+math.atan(top/bottom)))
def m(a,q):
    top=x3(a,q)**2+a**2-f(a,q)**2
    bottom=2*x3(a,q)*a
    return math.acos(top/bottom)
def phi(a,q):
    return (math.radians(180)-(m(a,q)+to(a,q)))

def x(a,q):
    return a*math.tan(to(a,q))

def x2(a,q):
    _to=(to(a,q))
    return math.sin(_to)*f(a,q)

def x3(a,q):
    return math.sqrt((f(a,q)**2)+a**2-(2*a*f(a,q)*math.cos((to(a,q)))))


print ("x=",(x3(15,12.8)))
print ("g=",math.degrees(to(15,12.8)))
print ("F=",(f(15,12.8)))
